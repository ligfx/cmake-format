/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <functional>

#include "helpers.h"
#include "transform.h"

using namespace std::placeholders;

static void delete_span(std::vector<Command> &commands, std::vector<Span> &spans,
                        size_t span_index) {
    spans.erase(spans.begin() + span_index);
    for (auto &c : commands) {
        if (c.identifier > span_index) {
            c.identifier--;
        }
    }
}

static void insert_before(size_t &span_index, std::vector<Command> &commands,
                          std::vector<Span> &spans, const std::vector<Span> &new_spans) {
    spans.insert(spans.begin() + span_index, new_spans.begin(), new_spans.end());
    for (auto &c : commands) {
        if (c.identifier > span_index) {
            c.identifier += new_spans.size();
        }
    }
    span_index += new_spans.size();
}

static void insert_before(size_t &span_index, std::vector<Command> &commands,
                          std::vector<Span> &spans, const Span &new_span) {
    return insert_before(span_index, commands, spans, std::vector<Span>{new_span});
}

static std::string get_command_indentation(const size_t &identifier_span_index,
                                           std::vector<Span> &spans) {
    const std::string ident = spans[identifier_span_index].data;

    if (identifier_span_index == 0 || spans[identifier_span_index - 1].type == SpanType::Newline) {
        return "";
    } else if (spans[identifier_span_index - 1].type == SpanType::Space) {
        return spans[identifier_span_index - 1].data;
    } else {
        throw std::runtime_error("command '" + ident + "' not preceded by space or newline: '" +
                                 spans[identifier_span_index - 1].data + "'");
    }
}

static void run(std::vector<Command> &commands, std::vector<Span> &spans, size_t column_width,
                const std::string &argument_indent_string) {

    (void)column_width;
    (void)argument_indent_string;

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
                insert_before(next_token, commands, spans,
                              {
                                  {SpanType::Newline, "\n"},
                                  {SpanType::Space, command_indentation + argument_indent_string},
                              });
                next_token++;
                line_width = column_width;
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
                if (line_width + argument_size <= column_width) {
                    if (!first_argument) {
                        insert_before(next_token, commands, spans, {SpanType::Space, " "});
                    }
                    line_width += argument_size;
                    first_argument = false;
                } else {
                    insert_before(
                        next_token, commands, spans,
                        {{SpanType::Newline, "\n"},
                         {SpanType::Space, command_indentation + argument_indent_string}});
                    line_width = command_indentation.size() + argument_indent_string.size() +
                                 argument_size - 1;
                }

                if (add_line_break) {
                    line_width = column_width;
                }

                next_token += argument_spans_count;
            } else {
                throw std::runtime_error("unexpected '" + spans[next_token].data + "'");
            }
        }
    }
}

static bool handleCommandLine(const std::string &arg,
                              std::vector<TransformFunction> &transform_functions) {
    if (arg.find("-argument-bin-pack=") != 0) {
        return false;
    }

    const size_t width = std::stoi(arg.substr(std::string{"-argument-bin-pack="}.size()));
    // TODO: allow specifying indent as well as column width
    transform_functions.emplace_back(std::bind(run, _1, _2, width, "    "));

    return true;
};

static const on_program_load transform_argument_per_line{[]() {
    getCommandLineDescriptions().emplace_back(
        "-argument-bin-pack=WIDTH",
        "\"Bin pack\" arguments, with a maximum column width of WIDTH.");
    getCommandLineHandlers().emplace_back(&handleCommandLine);
}};

TEST_CASE("Bin packs arguments", "[transform.argument_bin_pack]") {
    REQUIRE_TRANSFORMS_TO(std::bind(run, _1, _2, 30, "    "),
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
)");
}
