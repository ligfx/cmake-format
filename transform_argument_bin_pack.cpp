/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_argument_bin_pack(
    std::vector<Span> &spans, size_t column_limit, const std::string &argument_indent_string) {

    size_t current_index = 0;
    while (current_index < spans.size()) {
        if (spans[current_index].type != SpanType::CommandIdentifier) {
            current_index++;
            continue;
        }
        const size_t identifier_index = current_index;

        const std::string ident = lowerstring(spans[identifier_index].data);
        std::string command_indentation = get_command_indentation(identifier_index, spans);
        size_t line_width = command_indentation.size() + ident.size();

        current_index++;
        if (spans[current_index].type == SpanType::Space) {
            line_width += spans[current_index].data.size();
            current_index++;
        }
        if (spans[current_index].type != SpanType::Lparen) {
            throw std::runtime_error("expected lparen, got '" + spans[current_index].data + "'");
        }
        line_width += spans[current_index].data.size();
        current_index++;

        bool first_argument = true;
        while (spans[current_index].type != SpanType::Rparen) {
            if (spans[current_index].type == SpanType::Space) {
                delete_span(spans, current_index);
            } else if (spans[current_index].type == SpanType::Newline) {
                delete_span(spans, current_index);
            } else if (spans[current_index].type == SpanType::Comment) {
                insert_span_before(current_index, spans,
                    {
                        {SpanType::Newline, "\n"},
                        {SpanType::Space, command_indentation + argument_indent_string},
                    });
                current_index++;
                line_width = column_limit;
            } else if (spans[current_index].type == SpanType::Quoted ||
                       spans[current_index].type == SpanType::Unquoted) {

                bool add_line_break = false;
                size_t argument_spans_count = 1;
                if (spans[current_index + 1].type == SpanType::Comment) {
                    argument_spans_count = 2;
                    add_line_break = true;
                } else if (spans[current_index + 1].type == SpanType::Space &&
                           spans[current_index + 2].type == SpanType::Comment) {
                    argument_spans_count = 3;
                    add_line_break = true;
                }

                size_t argument_size = 0;
                for (size_t i = 0; i < argument_spans_count; i++) {
                    argument_size += spans[current_index + i].data.size();
                }

                if (!first_argument) {
                    argument_size += 1;
                }
                if (line_width + argument_size <= column_limit) {
                    if (!first_argument) {
                        insert_span_before(current_index, spans, {SpanType::Space, " "});
                    }
                    line_width += argument_size;
                    first_argument = false;
                } else {
                    insert_span_before(current_index, spans,
                        {{SpanType::Newline, "\n"},
                            {SpanType::Space, command_indentation + argument_indent_string}});
                    line_width = command_indentation.size() + argument_indent_string.size() +
                                 argument_size - 1;
                }

                if (add_line_break) {
                    line_width = column_limit;
                }

                current_index += argument_spans_count;
            } else {
                throw std::runtime_error("unexpected '" + spans[current_index].data + "'");
            }
        }

        if (line_width + 1 >= column_limit) {
            insert_span_before(current_index, spans,
                {{SpanType::Newline, "\n"},
                    {SpanType::Space, command_indentation + argument_indent_string}});
        }
        current_index++;
    }
}

TEST_CASE("Bin packs arguments") {
    REQUIRE_TRANSFORMS_TO(
        R"(
command(ARG1 ARG2 ARG3 ARG4 ARG5 ARG6
    ARG7)

command(
    ARG1
    ARG2
    ARG3 ARG4
    ARG5 ARG6
    ARG7 ARG8 ARG9 ARG10)

command(
    ARG1# comment
    # entire line comment
    ARG2 # comment preceded by space
    ARG3 #a
    ARG4
    ARG5 ARG6
    ARG7 ARG8 ARG9 ARG10)
)",
        R"(
command(ARG1 ARG2 ARG3 ARG4
    ARG5 ARG6 ARG7)

command(ARG1 ARG2 ARG3 ARG4
    ARG5 ARG6 ARG7 ARG8 ARG9
    ARG10)

command(ARG1# comment
    # entire line comment
    ARG2 # comment preceded by space
    ARG3 #a
    ARG4 ARG5 ARG6 ARG7 ARG8
    ARG9 ARG10)
)",
        transform_argument_bin_pack, 30, "    ");
}

TEST_CASE("Puts line break between line-comment and closing paren") {
    REQUIRE_TRANSFORMS_TO(R"(
command(
    #comment
)
)",
        R"(
command(
    #comment
    )
)",
        transform_argument_bin_pack, 30, "    ");
}

TEST_CASE("Puts line break between arg-comment and closing paren") {
    REQUIRE_TRANSFORMS_TO(
        R"(
command(ARG # comment
)
)",
        R"(
command(ARG # comment
    )
)",
        transform_argument_bin_pack, 30, "    ");
}
