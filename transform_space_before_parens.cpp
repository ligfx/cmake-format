/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "transform.h"

void transform_space_before_parens(
    std::vector<Span> &spans, SpaceBeforeParens space_before_parens) {

    size_t current_index = 0;
    while (current_index < spans.size()) {
        if (spans[current_index].type == SpanType::CommandIdentifier) {
            // Trying something new: iterate commands just by identifier tokens.

            const std::string ident = lowerstring(spans[current_index].data);

            bool want_space = false;
            if (space_before_parens == SpaceBeforeParens::Always) {
                want_space = true;
            }
            if (space_before_parens == SpaceBeforeParens::ControlStatements &&
                (ident == "if" || ident == "elseif" || ident == "endif" || ident == "macro" ||
                    ident == "endmacro" || ident == "function" || ident == "endfunction" ||
                    ident == "foreach" || ident == "endforeach" || ident == "while" ||
                    ident == "endwhile")) {
                want_space = true;
            }

            if (spans[current_index + 1].type == SpanType::Space) {
                if (want_space) {
                    spans[current_index + 1].data = " ";
                } else {
                    delete_span(spans, current_index + 1);
                }
            } else if (want_space) {
                insert_span_at(current_index + 1, spans, {SpanType::Space, " "});
            }
        }
        current_index++;
    }
}

TEST_CASE("Puts space before parens") {
    REQUIRE_TRANSFORMS_TO(
        R"(
command()
command  ()
)",
        R"(
command ()
command ()
)",
        transform_space_before_parens, SpaceBeforeParens::Always);
}

TEST_CASE("Removes space before parens") {
    REQUIRE_TRANSFORMS_TO(
        R"(
command()
command  ()
)",
        R"(
command()
command()
)",
        transform_space_before_parens, SpaceBeforeParens::Never);
}

TEST_CASE("Removes space before parens for non-control statements") {
    REQUIRE_TRANSFORMS_TO(
        R"(
command ()
if ()
endif()
)",
        R"(
command()
if ()
endif ()
)",
        transform_space_before_parens, SpaceBeforeParens::ControlStatements);
}
