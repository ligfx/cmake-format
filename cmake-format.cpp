/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "command_line.h"
#include "helpers.h"
#include "parser.h"
#include "transform.h"

struct inputwrapper {
    inputwrapper() : standard{true} {
    }
    void open(const std::string &filename) {
        standard = false;
        file.open(filename);
    }

    operator std::istream &() {
        if (standard) {
            return std::cin;
        } else {
            return file;
        }
    }

    bool standard;
    std::ifstream file;
};

struct outputwrapper {
    outputwrapper() : standard{true} {
    }
    void open(const std::string &filename) {
        standard = false;
        file.open(filename);
    }

    operator std::ostream &() {
        if (standard) {
            return std::cout;
        } else {
            return file;
        }
    }

    bool standard;
    std::ofstream file;
};

enum class ReflowArguments {
    None,
    OnePerLine,
    BinPack,
    Heuristic,
};

int main(int argc, char **argv) {

    size_t column_limit{80};
    LetterCase command_case{LetterCase::Lower};
    size_t continuation_indent_width{0};
    size_t indent_width{4};
    size_t max_empty_lines_to_keep{1};
    ReflowArguments reflow_arguments{ReflowArguments::None};
    SpaceBeforeParens space_before_parens{SpaceBeforeParens::Never};

    bool quiet = false;
    bool format_in_place = false;

    const static std::string description =
        "Re-formats specified files. If no files are specified on the command-line,\n"
        "reads from standard input. If -i is specified, formats files in-place;\n"
        "otherwise, writes results to standard output.";

    const static std::vector<SwitchOptionDescription> switch_options = {
        {"-i", "Re-format files in-place.", format_in_place},
        {"-q", "Quiet mode: suppress informational messages.", quiet},
    };

    const static std::vector<ArgumentOptionDescription> argument_options = {
        {"-column-limit", "NUMBER",
            "Set maximum column width to NUMBER. If ReflowArguments is None, this does nothing.",
            parse_numeric_option(column_limit)},
        {"-command-case", "CASE", "Letter case of command invocations. Available: lower, upper",
            [&](const std::string &value) {
                if (value == "lower") {
                    command_case = LetterCase::Lower;
                } else if (value == "upper") {
                    command_case = LetterCase::Upper;
                } else {
                    throw opterror;
                }
            }},
        {"-continuation-indent-width", "NUMBER", "Indent width for line continuations.",
            parse_numeric_option(continuation_indent_width)},
        {"-indent-width", "NUMBER", "Use NUMBER spaces for indentation.",
            parse_numeric_option(indent_width)},
        {"-max-empty-lines-to-keep", "NUMBER",
            "The maximum number of consecutive empty lines to keep.",
            parse_numeric_option(max_empty_lines_to_keep)},
        {"-reflow-arguments", "ALGORITHM",
            "Algorithm to reflow command arguments. Available: none, oneperline, binpack, "
            "heuristic",
            [&](const std::string &value) {
                if (value == "none") {
                    reflow_arguments = ReflowArguments::None;
                } else if (value == "oneperline") {
                    reflow_arguments = ReflowArguments::OnePerLine;
                } else if (value == "binpack") {
                    reflow_arguments = ReflowArguments::BinPack;
                } else if (value == "heuristic") {
                    reflow_arguments = ReflowArguments::Heuristic;
                } else {
                    throw opterror;
                }
            }},
        {"-space-before-parens", "CONDITION",
            "When to put a space before opening parentheses. Available: always, controlstatements, "
            "never",
            [&](const std::string &value) {
                if (value == "always") {
                    space_before_parens = SpaceBeforeParens::Always;
                } else if (value == "controlstatements") {
                    space_before_parens = SpaceBeforeParens::ControlStatements;
                } else if (value == "never") {
                    space_before_parens = SpaceBeforeParens::Never;
                } else {
                    throw opterror;
                }
            }},

        // {"-indent-rparen=STRING", "Use STRING for indenting hanging right-parens."},
        // {"-argument-per-line=STRING", "Put each argument on its own line, indented by STRING."},
        // {"-argument-bin-pack=WIDTH", "\"Bin pack\" arguments, with a maximum column width of
        // WIDTH."},
    };

    std::vector<std::string> filenames =
        parse_command_line(argc, argv, description, switch_options, argument_options);

    if (continuation_indent_width == 0) {
        continuation_indent_width = indent_width;
    }

    if (filenames.size() == 0) {
        if (format_in_place) {
            fprintf(stderr, "%s: '-i' specified without any filenames. Try: %s -help\n", argv[0],
                argv[0]);
            exit(1);
        }
        if (!quiet) {
            fprintf(stderr,
                "%s: no filenames specified, reading from stdin and writing to stdout...\n",
                argv[0]);
        }
        filenames.emplace_back("-");
    }

    for (auto filename : filenames) {
        inputwrapper file_in;
        if (filename != "-") {
            file_in.open(filename);
        };
        std::string content;
        { content = {std::istreambuf_iterator<char>(file_in), std::istreambuf_iterator<char>()}; }

        std::vector<Span> spans = parse(content);

        transform_indent(spans, repeat_string(" ", indent_width));

        if (reflow_arguments == ReflowArguments::BinPack) {
            transform_argument_bin_pack(
                spans, column_limit, repeat_string(" ", continuation_indent_width));
        } else if (reflow_arguments == ReflowArguments::OnePerLine) {
            transform_argument_per_line(spans, repeat_string(" ", continuation_indent_width));
        } else if (reflow_arguments == ReflowArguments::Heuristic) {
            transform_argument_heuristic(
                spans, column_limit, repeat_string(" ", continuation_indent_width));
        }
        transform_command_case(spans, command_case);
        transform_squash_empty_lines(spans, max_empty_lines_to_keep);
        transform_space_before_parens(spans, space_before_parens);

        outputwrapper file_out;
        if (format_in_place && filename != "-") {
            file_out.open(filename);
        }
        for (auto s : spans) {
            (std::ostream &)file_out << s.data;
        }
    }
}
