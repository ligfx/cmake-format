/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_indent(std::vector<Span> &spans, const std::string &indent_string) {

    int global_indentation_level = 0;

    size_t current_index = 0;
    while (current_index < spans.size()) {
        if (spans[current_index].type != SpanType::CommandIdentifier) {
            current_index++;
            continue;
        }
        const size_t identifier_index = current_index;
        const std::string ident = lowerstring(spans[identifier_index].data);

        if (ident == "endif" || ident == "endforeach" || ident == "endwhile" ||
            ident == "endmacro" || ident == "endfunction") {
            global_indentation_level--;
        }

        auto indentation_level = global_indentation_level;
        if (ident == "else" || ident == "elseif") {
            indentation_level--;
        }

        const std::string old_indentation = spans[identifier_index - 1].data;

        // Re-indent the command invocation
        spans[identifier_index - 1].data = repeat_string(indent_string, indentation_level);

        // Walk forwards to fix arguments and the closing paren.
        current_index++;
        while (true) {
            if (spans[current_index].type == SpanType::Newline) {
                size_t old_indentation_pos = spans[current_index + 1].data.find(old_indentation);
                if (old_indentation_pos != 0) {
                } else {
                    spans[current_index + 1].data =
                        (repeat_string(indent_string, indentation_level) +
                            spans[current_index + 1].data.substr(old_indentation.size()));
                }
                current_index++;
            } else if (spans[current_index].type == SpanType::Rparen) {
                break;
            } else {
                current_index++;
            }
        }

        // Walk backwards to fix comments at the same prior indentation
        // level.
        // TODO: use iterators instead of fragile integer indices?
        std::ptrdiff_t last_token_on_previous_line = identifier_index - 3;
        while (last_token_on_previous_line >= 0) {
            if (last_token_on_previous_line >= 2 &&
                spans[last_token_on_previous_line].type == SpanType::Comment &&
                spans[last_token_on_previous_line - 1].type == SpanType::Space &&
                spans[last_token_on_previous_line - 2].type == SpanType::Newline) {
                spans[last_token_on_previous_line - 1].data =
                    repeat_string(indent_string, indentation_level);
                last_token_on_previous_line -= 3;
            } else if (last_token_on_previous_line >= 1 &&
                       spans[last_token_on_previous_line].type == SpanType::Space &&
                       spans[last_token_on_previous_line - 1].type == SpanType::Newline) {
                last_token_on_previous_line -= 2;
            } else if (spans[last_token_on_previous_line].type == SpanType::Newline) {
                last_token_on_previous_line -= 1;
            } else {
                break;
            }
        }

        if (ident == "if" || ident == "foreach" || ident == "while" || ident == "macro" ||
            ident == "function") {
            global_indentation_level++;
        }
        current_index++;
    }
}

TEST_CASE("Reindents toplevel") {
    REQUIRE_TRANSFORMS_TO(
        R"(
   improperly_indented_toplevel()
correctly_indented_toplevel()
)",
        R"(
improperly_indented_toplevel()
correctly_indented_toplevel()
)",
        transform_indent, "INDENT ");
}

TEST_CASE("Reindents arguments") {
    REQUIRE_TRANSFORMS_TO(
        R"(
    command(
       ARGUMENT
           ANOTHER_ARGUMENT
    )
if()
command(
ARGUMENT
)
endif()
)",
        R"(
command(
   ARGUMENT
       ANOTHER_ARGUMENT
)
if()
INDENT command(
INDENT ARGUMENT
INDENT )
endif()
)",
        transform_indent, "INDENT ");
}

TEST_CASE("Reindents comments") {
    REQUIRE_TRANSFORMS_TO(
        R"(
    # associated comment
    # with multiple lines
    command()
)",
        R"(
# associated comment
# with multiple lines
command()
)",
        transform_indent, "INDENT ");
}

TEST_CASE("Reindents blocks") {
    REQUIRE_TRANSFORMS_TO(
        R"(
   if(CONDITION)
command()
       if(ANOTHER_CONDITION)
command()
endif()
           elseif(YET_ANOTHER_CONDITION)
  else(SHOULD_NOT_BE_INDENTED)
command()
endif()

       foreach()
command()
endforeach()

   macro()
command()
   endmacro()

while()
command()
endwhile()
)",
        R"(
if(CONDITION)
INDENT command()
INDENT if(ANOTHER_CONDITION)
INDENT INDENT command()
INDENT endif()
elseif(YET_ANOTHER_CONDITION)
else(SHOULD_NOT_BE_INDENTED)
INDENT command()
endif()

foreach()
INDENT command()
endforeach()

macro()
INDENT command()
endmacro()

while()
INDENT command()
endwhile()
)",
        transform_indent, "INDENT ");
}
