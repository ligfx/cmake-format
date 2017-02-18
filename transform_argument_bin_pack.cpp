/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_argument_bin_pack(std::vector<Command> &commands, std::vector<Span> &spans,
    size_t column_limit, const std::string &argument_indent_string) {

    for (auto c : commands) {
        const std::string ident = lowerstring(spans[c.identifier].data);

        std::string command_indentation = get_command_indentation(c.identifier, spans);
        size_t line_width = command_indentation.size() + ident.size();

        size_t next_token = c.identifier + 1;
        if (spans[next_token].type == SpanType::Space) {
            line_width += spans[next_token].data.size();
            next_token++;
        }
        if (spans[next_token].type != SpanType::Lparen) {
            throw std::runtime_error("expected lparen, got '" + spans[next_token].data + "'");
        }
        line_width += spans[next_token].data.size();
        next_token++;

        bool first_argument = true;
        while (spans[next_token].type != SpanType::Rparen) {
            if (spans[next_token].type == SpanType::Space) {
                delete_span(commands, spans, next_token);
            } else if (spans[next_token].type == SpanType::Newline) {
                delete_span(commands, spans, next_token);
            } else if (spans[next_token].type == SpanType::Comment) {
                insert_span_before(next_token, commands, spans,
                    {
                        {SpanType::Newline, "\n"},
                        {SpanType::Space, command_indentation + argument_indent_string},
                    });
                next_token++;
                line_width = column_limit;
            } else if (spans[next_token].type == SpanType::Quoted ||
                       spans[next_token].type == SpanType::Unquoted) {

                bool add_line_break = false;
                size_t argument_spans_count = 1;
                if (spans[next_token + 1].type == SpanType::Comment) {
                    argument_spans_count = 2;
                    add_line_break = true;
                } else if (spans[next_token + 1].type == SpanType::Space &&
                           spans[next_token + 2].type == SpanType::Comment) {
                    argument_spans_count = 3;
                    add_line_break = true;
                }

                size_t argument_size = 0;
                for (size_t i = 0; i < argument_spans_count; i++) {
                    argument_size += spans[next_token + i].data.size();
                }

                if (!first_argument) {
                    argument_size += 1;
                }
                if (line_width + argument_size <= column_limit) {
                    if (!first_argument) {
                        insert_span_before(next_token, commands, spans, {SpanType::Space, " "});
                    }
                    line_width += argument_size;
                    first_argument = false;
                } else {
                    insert_span_before(next_token, commands, spans,
                        {{SpanType::Newline, "\n"},
                            {SpanType::Space, command_indentation + argument_indent_string}});
                    line_width = command_indentation.size() + argument_indent_string.size() +
                                 argument_size - 1;
                }

                if (add_line_break) {
                    line_width = column_limit;
                }

                next_token += argument_spans_count;
            } else {
                throw std::runtime_error("unexpected '" + spans[next_token].data + "'");
            }
        }

        if (line_width + 1 >= column_limit) {
            insert_span_before(next_token, commands, spans,
                {{SpanType::Newline, "\n"},
                    {SpanType::Space, command_indentation + argument_indent_string}});
        }
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
