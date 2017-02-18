/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_argument_per_line(
    std::vector<Span> &spans, const std::string &argument_indent_string) {

    size_t next_token = 0;
    while (next_token < spans.size()) {
        if (spans[next_token].type != SpanType::CommandIdentifier) {
            next_token++;
            continue;
        }
        const size_t identifier_index = next_token;
        std::string command_indentation = get_command_indentation(identifier_index, spans);

        // Walk forwards to fix argument indents.
        next_token++;
        while (spans[next_token].type != SpanType::Lparen) {
            next_token++;
        }
        next_token++;
        while (spans[next_token].type != SpanType::Rparen) {
            if (spans[next_token].type == SpanType::Space) {
                delete_span(spans, next_token);
            } else if (spans[next_token].type == SpanType::Newline) {
                delete_span(spans, next_token);
            } else {
                insert_span_before(next_token, spans,
                    {{SpanType::Newline, "\n"},
                        {SpanType::Space, command_indentation + argument_indent_string}});
                next_token++;
            }
        }
        insert_span_before(
            next_token, spans, {{SpanType::Newline, "\n"}, {SpanType::Space, command_indentation}});
        next_token++;
    }
}

TEST_CASE("Puts each argument on its own line") {
    REQUIRE_TRANSFORMS_TO(
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
)",
        transform_argument_per_line, "INDENT ");
}
