/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <functional>

#include "helpers.h"
#include "transform.h"

using namespace std::placeholders;

static void run(const std::vector<Command> &commands, std::vector<Span> &spans,
                const std::string &rparen_indent_string) {
    for (auto c : commands) {
        const std::string ident = lowerstring(spans[c.identifier].data);

        std::string command_indentation;
        if (spans[c.identifier - 1].type == SpanType::Newline) {
            command_indentation = "";
        } else {
            command_indentation = spans[c.identifier - 1].data;
        }

        // Walk forwards to fix continuation indents.
        size_t next_token = c.identifier + 1;
        while (true) {
            if (spans[next_token].type == SpanType::Rparen) {
                if (spans[next_token - 2].type == SpanType::Newline) {
                    spans[next_token - 1].data = (command_indentation + rparen_indent_string);
                }
                break;
            } else {
                next_token++;
            }
        }
    }
}

static bool handleCommandLine(const std::string &arg,
                              std::vector<TransformFunction> &tranform_functions) {
    if (arg.find("-indent-rparen=") != 0) {
        return false;
    }

    std::string rparen_indent_string = arg.substr(std::string{"-indent-rparen="}.size());
    replace_all_in_string(rparen_indent_string, "\\t", "\t");

    tranform_functions.emplace_back(std::bind(run, _1, _2, rparen_indent_string));

    return true;
}

static const on_program_load transform_argument_per_line{[]() {
    getCommandLineDescriptions().emplace_back("-indent-rparen=STRING",
                                              "Use STRING for indenting hanging right-parens.");

    getCommandLineHandlers().emplace_back(&handleCommandLine);
}};
