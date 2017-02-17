/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_argument_per_line(std::vector<Command> &commands, std::vector<Span> &spans,
    const std::string &argument_indent_string) {

    for (auto c : commands) {
        std::string command_indentation = get_command_indentation(c.identifier, spans);

        // Walk forwards to fix argument indents.
        size_t next_token = c.identifier + 1;
        while (spans[next_token].type != SpanType::Lparen) {
            next_token++;
        }
        next_token++;
        while (spans[next_token].type != SpanType::Rparen) {
            if (spans[next_token].type == SpanType::Space) {
                delete_span(commands, spans, next_token);
            } else if (spans[next_token].type == SpanType::Newline) {
                delete_span(commands, spans, next_token);
            } else {
                insert_span_before(next_token, commands, spans,
                    {{SpanType::Newline, "\n"},
                        {SpanType::Space, command_indentation + argument_indent_string}});
                next_token++;
            }
        }
        insert_span_before(next_token, commands, spans,
            {{SpanType::Newline, "\n"}, {SpanType::Space, command_indentation}});
    }
}

TEST_CASE("Puts each argument on its own line", "[transform.argument_per_line]") {
    REQUIRE_TRANSFORMS_TO(std::bind(transform_argument_per_line, _1, _2, "INDENT "),
        R"(
    command(ARG1 ARG2 ARG3)
    cmake_minimum_required(VERSION 3.0)
    project(cmake-format)
)",
        R"(
    command(
    INDENT ARG1
    INDENT ARG2
    INDENT ARG3
    )
    cmake_minimum_required(
    INDENT VERSION
    INDENT 3.0
    )
    project(
    INDENT cmake-format
    )
)");
}
