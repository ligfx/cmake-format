/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <functional>

#include "helpers.h"
#include "transform.h"

using namespace std::placeholders;

static void run(std::vector<Command> &commands, std::vector<Span> &spans,
                const std::string &argument_indent_string) {

    for (auto c : commands) {
        const std::string ident = lowerstring(spans[c.identifier].data);

        std::string command_indentation;
        if (spans[c.identifier - 1].type == SpanType::Newline) {
            command_indentation = "";
        } else if (spans[c.identifier - 1].type == SpanType::Space) {
            command_indentation = spans[c.identifier - 1].data;
        } else {
            throw std::runtime_error("command '" + ident + "' not preceded by space or newline: '" +
                                     spans[c.identifier - 1].data + "'");
        }

        // Walk forwards to fix argument indents.
        size_t next_token = c.identifier + 1;
        while (spans[next_token].type != SpanType::Lparen) {
            next_token++;
        }
        next_token++;
        while (spans[next_token].type != SpanType::Rparen) {
            if (spans[next_token].type == SpanType::Space) {
                spans[next_token].data = "";
            } else if (spans[next_token].type == SpanType::Newline) {
                spans[next_token].data = "";
            } else {
                if (spans[next_token - 1].type != SpanType::Space) {
                    spans.insert(spans.begin() + next_token, Span{SpanType::Space, ""});
                    next_token++;
                    // Fix up commands
                    for (auto &c : commands) {
                        if (c.identifier >= next_token) {
                            c.identifier++;
                        }
                    }
                }
                spans[next_token - 1].data = "\n" + command_indentation + argument_indent_string;
            }
            next_token++;
        }
        if (spans[next_token - 1].type != SpanType::Space) {
            spans.insert(spans.begin() + next_token, Span{SpanType::Space, ""});
            next_token++;
            // Fix up commands
            for (auto &c : commands) {
                if (c.identifier >= next_token) {
                    c.identifier++;
                }
            }
        }
        spans[next_token - 1].data = "\n" + command_indentation;
    }
}

static bool handleCommandLine(const std::string &arg,
                              std::vector<TransformFunction> &transform_functions) {
    if (arg.find("-argument-per-line=") != 0) {
        return false;
    }

    std::string indent_string = arg.substr(std::string{"-argument-per-line="}.size());
    replace_all_in_string(indent_string, "\\t", "\t");
    transform_functions.emplace_back(std::bind(run, _1, _2, indent_string));

    return true;
};

static const on_program_load transform_argument_per_line{[]() {
    getCommandLineDescriptions().emplace_back(
        "-argument-per-line=STRING", "Put each argument on its own line, indented by STRING.");
    getCommandLineHandlers().emplace_back(&handleCommandLine);
}};

TEST_CASE("Puts each argument on its own line", "[transform.argument_per_line]") {
    REQUIRE_TRANSFORMS_TO(std::bind(run, _1, _2, "INDENT "),
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
