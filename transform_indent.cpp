/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <functional>

#include "helpers.h"

using namespace std::placeholders;

void run(const std::vector<Command> &commands, std::vector<Span> &spans,
    const std::string &indent_string) {
    int global_indentation_level = 0;
    for (auto c : commands) {
        const std::string ident = lowerstring(spans[c.identifier].data);

        if (ident == "endif" || ident == "endforeach" || ident == "endwhile" ||
            ident == "endmacro" || ident == "endfunction") {
            global_indentation_level--;
        }

        auto indentation_level = global_indentation_level;
        if (ident == "else" || ident == "elseif") {
            indentation_level--;
        }

        const std::string old_indentation = spans[c.identifier - 1].data;

        // Re-indent the command invocation
        spans[c.identifier - 1].data = repeat_string(indent_string, indentation_level);

        // Walk forwards to fix arguments and the closing paren.
        size_t next_token = c.identifier + 1;
        while (true) {
            if (spans[next_token].type == SpanType::Newline) {
                size_t old_indentation_pos = spans[next_token + 1].data.find(old_indentation);
                if (old_indentation_pos != 0) {
                } else {
                    spans[next_token + 1].data =
                        (repeat_string(indent_string, indentation_level) +
                            spans[next_token + 1].data.substr(old_indentation.size()));
                }
                next_token++;
            } else if (spans[next_token].type == SpanType::Rparen) {
                break;
            } else {
                next_token++;
            }
        }

        // Walk backwards to fix comments at the same prior indentation
        // level.
        // TODO: use iterators instead of fragile integer indices?
        std::ptrdiff_t last_token_on_previous_line = c.identifier - 3;
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
    }
}

bool handleCommandLine(
    const std::string &arg, std::vector<TransformFunction> &transform_functions) {
    if (arg.find("-indent=") != 0) {
        return false;
    }

    std::string indent_string = arg.substr(std::string{"-indent="}.size());
    replace_all_in_string(indent_string, "\\t", "\t");
    transform_functions.emplace_back(std::bind(run, _1, _2, indent_string));

    return true;
};

static const on_program_load transform_argument_per_line{[]() {
    getCommandLineDescriptions().emplace_back("-indent=STRING", "Use STRING for indentation.");
    getCommandLineHandlers().emplace_back(&handleCommandLine);
}};

TEST_CASE("Reindents toplevel", "[transform.indent]") {
    REQUIRE_TRANSFORMS_TO(std::bind(run, _1, _2, "INDENT "),
        R"(
   improperly_indented_toplevel()
correctly_indented_toplevel()
)",
        R"(
improperly_indented_toplevel()
correctly_indented_toplevel()
)");
}

TEST_CASE("Reindents arguments", "[transform.indent]") {
    REQUIRE_TRANSFORMS_TO(std::bind(run, _1, _2, "INDENT "),
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
)");
}

TEST_CASE("Reindents comments", "[transform.indent]") {
    REQUIRE_TRANSFORMS_TO(std::bind(run, _1, _2, "INDENT "),
        R"(
    # associated comment
    # with multiple lines
    command()
)",
        R"(
# associated comment
# with multiple lines
command()
)");
}

TEST_CASE("Reindents blocks", "[transform.indent]") {
    REQUIRE_TRANSFORMS_TO(std::bind(run, _1, _2, "INDENT "),
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
)");
}
