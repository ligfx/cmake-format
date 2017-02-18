/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_argument_per_line(
    std::vector<Span> &spans, const std::string &argument_indent_string) {

    size_t current_index = 0;
    while (current_index < spans.size()) {
        if (spans[current_index].type != SpanType::CommandIdentifier) {
            current_index++;
            continue;
        }
        const size_t identifier_index = current_index;
        std::string command_indentation = get_command_indentation(identifier_index, spans);

        // Walk forwards to fix argument indents.
        current_index++;
        while (spans[current_index].type != SpanType::Lparen) {
            current_index++;
        }
        current_index++;
        while (spans[current_index].type != SpanType::Rparen) {
            if (spans[current_index].type == SpanType::Space) {
                delete_span(spans, current_index);
            } else if (spans[current_index].type == SpanType::Newline) {
                delete_span(spans, current_index);
            } else {
                insert_span_before(current_index, spans,
                    {{SpanType::Newline, "\n"},
                        {SpanType::Space, command_indentation + argument_indent_string}});
                current_index++;
            }
        }
        insert_span_before(current_index, spans,
            {{SpanType::Newline, "\n"}, {SpanType::Space, command_indentation}});
        current_index++;
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
