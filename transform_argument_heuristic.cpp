/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <cstdio>
#include <functional>

#include <optional.hpp>

#include "helpers.h"
#include "transform.h"

using std::experimental::optional;
using std::experimental::nullopt;
using namespace std::placeholders;

using ThreeArgumentWindowFunc = std::function<void(size_t, optional<size_t>, optional<size_t>)>;
static void inline three_argument_window(const size_t &identifier_span_index,
    std::vector<Span> &spans, const ThreeArgumentWindowFunc &f) {
    size_t arg3 = identifier_span_index + 1;
    while (spans[arg3].type != SpanType::Lparen) {
        arg3++;
    }
    arg3++;
    optional<size_t> arg1;
    optional<size_t> arg2;
    while (spans[arg3].type != SpanType::Rparen) {
        if (spans[arg3].type == SpanType::Quoted || spans[arg3].type == SpanType::Unquoted) {
            if (arg1) {
                f(*arg1, arg2, arg3);
            }

            arg1 = arg2;
            arg2 = arg3;
        }
        arg3++;
    }
    if (arg1) {
        f(*arg1, arg2, nullopt);
    }
    if (arg2) {
        f(*arg2, nullopt, nullopt);
    }
}

static bool is_not_command_option(const std::string &value) {
    return std::any_of(value.begin(), value.end(),
        [](char c) { return !std::isupper(c) && c != '_' && c != '-' && !std::isdigit(c); });
}

void transform_argument_heuristic(
    std::vector<Span> &spans, size_t column_width, const std::string &argument_indent_string) {

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

        std::vector<size_t> widths;
        {
            bool run_of_three_lowercase = false;
            bool blacklisted_keyword = false;
            size_t argument_ordinal = 0;
            auto f = [&](
                const size_t &arg1, const optional<size_t> &arg2, const optional<size_t> &arg3) {
                if (spans[arg1].data == "COMMAND") {
                    blacklisted_keyword = true;
                }
                if (blacklisted_keyword) {
                } else if (is_not_command_option(spans[arg1].data)) {
                    if (arg2 && arg3 && is_not_command_option(spans[*arg2].data) &&
                        is_not_command_option(spans[*arg3].data)) {
                        run_of_three_lowercase = true;
                    }
                } else {
                    run_of_three_lowercase = false;
                    blacklisted_keyword = false;
                }
                if ((ident == "add_executable" || ident == "add_library") &&
                    argument_ordinal == 0) {
                    widths.push_back(0);
                } else if (run_of_three_lowercase) {
                    widths.push_back(column_width);
                } else {
                    widths.push_back(0);
                }
                argument_ordinal++;
            };
            three_argument_window(identifier_index, spans, f);
        }

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

        size_t argument_ordinal = 0;
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
                line_width = column_width;
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
                if (argument_ordinal != 0) {
                    argument_size += 1;
                }
                argument_size += widths[argument_ordinal];

                if (line_width + argument_size <= column_width) {
                    if (argument_ordinal != 0) {
                        insert_span_before(current_index, spans, {SpanType::Space, " "});
                    }
                    line_width += argument_size;
                } else {
                    insert_span_before(current_index, spans,
                        {{SpanType::Newline, "\n"},
                            {SpanType::Space, command_indentation + argument_indent_string}});
                    line_width = command_indentation.size() + argument_indent_string.size() +
                                 argument_size - 1;
                }

                if (add_line_break) {
                    line_width = column_width;
                }

                current_index += argument_spans_count;
                argument_ordinal++;
            } else {
                throw std::runtime_error("unexpected '" + spans[current_index].data + "'");
            }
        }

        if (line_width + 1 >= column_width) {
            insert_span_before(current_index, spans,
                {{SpanType::Newline, "\n"},
                    {SpanType::Space, command_indentation + argument_indent_string}});
        }

        current_index++;
    }
}

// TEST_CASE("Puts each argument on its own line", "[argument-heuristic]") {
//     REQUIRE_TRANSFORMS_TO(run,
//         R"(
//     command(ARG1 ARG2 ARG3)
//     cmake_minimum_required(VERSION 3.0)
//     project(cmake-format)
// )",
//         R"(
//     command(
//     INDENT ARG1
//     INDENT ARG2
//     INDENT ARG3
//     )
//     cmake_minimum_required(
//     INDENT VERSION
//     INDENT 3.0
//     )
//     project(
//     INDENT cmake-format
//     )
// )");
// }
