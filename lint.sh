#!/bin/bash

set -euo pipefail

exit_code=0

function indent() {
    while read line; do
        printf "  "
        echo "$line"
    done
}

function error() {
    exit_code=1
    echo >&2 -e "ERROR: $@"
}

lexer_source_commit=$(git --no-pager log -n 1 --pretty=format:%h -- 3rdparty/cmake-3.7.2/Source/cmListFileLexer.in.l)
lexer_generated_commit=$(git --no-pager log -n 1 --pretty=format:%h -- generated/cmListFileLexer.c)
git merge-base --is-ancestor "${lexer_source_commit}" "${lexer_generated_commit}" ||
    error "3rdparty/cmake-3.7.2/Source/cmListFileLexer.in.l is newer than generated/cmListFileLexer.c"

function enforce_license_header() {
    contents=$(cat)
    if ! echo "$contents" | head -n 1 | grep -q "BSD 3-Clause"; then
        echo "/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying"
        echo "   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for"
        echo "   details.  */"
        echo
    fi
    echo "$contents"
}

function enforce_c_style() {
    enforce_license_header | "${CLANG_FORMAT:-clang-format}"
}

CMAKE_FORMAT=""
if test -f cmake-format; then CMAKE_FORMAT="./cmake-format"; fi
if test -f build/cmake-format; then CMAKE_FORMAT="build/cmake-format"; fi
if ! test "${CMAKE_FORMAT}"; then
    error "Couldn't find executable 'cmake-format'"
fi
function enforce_cmake_style() {
    if test "${CMAKE_FORMAT}"; then
        "${CMAKE_FORMAT}" -indent="    " -indent-rparen="" -lowercase-commands
    else
        cat
    fi
}

if which colordiff >/dev/null; then
    COLORDIFF=colordiff
else
    COLORDIFF=cat
fi
function clean_diff_output() {
    sed -e "/^--- .*/d" -e "/^+++ .*/d" | $COLORDIFF
}

index_filenames=$(git diff --name-only --diff-filter=ACMRTUXB --cached)
while read filename; do
    if git check-ignore -q "$filename"; then
        continue
    fi

    if echo "$filename" | egrep -q "[.](c|h)(pp)?$"; then
        enforce_style=enforce_c_style
    elif echo "$filename" | egrep -q "/CMakeLists.txt$"; then
        enforce_style=enforce_cmake_style
    else
        continue
    fi

    filename=${filename#./}
    if echo "${index_filenames}" | egrep -q "^$filename$"; then
        index_contents=$(mktemp)
        formatted=$(mktemp)
        git show ":$filename" > "$index_contents"

        $enforce_style <"$index_contents" >"$formatted"
        if ! diff -u "$index_contents" "$formatted" >/dev/null; then
            error "Index copy of \033[31m$filename\033[0m has differences after formatting:"
            (diff -u "$index_contents" "$formatted" || true) | clean_diff_output | indent >&2
            echo >&2
        fi
    fi

    d=$($enforce_style < "$filename")
    if test "$d" != "$(cat "$filename")"; then
        echo "$d" > "$filename"
    fi
done < <(find . -not \( -path "./3rdparty" -prune -o -path "./generated" -prune \) -name '*.cpp' -o -name '*.h' -o -name 'CMakeLists.txt')

exit $exit_code
