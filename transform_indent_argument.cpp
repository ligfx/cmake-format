/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <functional>

#include "helpers.h"
#include "transform.h"

using namespace std::placeholders;

static void run(const std::vector<Command> &commands, std::vector<Span> &spans,
    bool argument_indent_after_lparen, std::string argument_indent_string) {

    (void)argument_indent_after_lparen;

    for (auto c : commands) {
        const std::string ident = lowerstring(spans[c.identifier].data);

        std::string command_indentation;
        if (spans[c.identifier - 1].type == SpanType::Newline) {
            command_indentation = "";
        } else {
            command_indentation = spans[c.identifier - 1].data;
        }

        if (argument_indent_after_lparen) {
            argument_indent_string = repeat_string(" ", ident.size() + 1);
        }

        // Walk forwards to fix argument indents.
        size_t next_token = c.identifier + 1;
        while (true) {
            if (spans[next_token].type == SpanType::Newline) {
                if (spans[next_token + 2].type != SpanType::Rparen) {
                    spans[next_token + 1].data = command_indentation + argument_indent_string;
                }
                next_token++;
            } else if (spans[next_token].type == SpanType::Rparen) {
                break;
            } else {
                next_token++;
            }
        }
    }
}

static bool handleCommandLine(
    const std::string &arg, std::vector<TransformFunction> &tranform_functions) {

    if (arg == "-indent-arguments=align-paren") {
        tranform_functions.emplace_back(std::bind(run, _1, _2, true, ""));
        return true;
    }
    if (arg.find("-indent-arguments=") == 0) {
        std::string argument_indent_string = arg.substr(std::string{"-indent-arguments="}.size());
        replace_all_in_string(argument_indent_string, "\\t", "\t");
        tranform_functions.emplace_back(std::bind(run, _1, _2, false, argument_indent_string));
        return true;
    }

    return false;
}

static const on_program_load transform_argument_per_line{[]() {
    getCommandLineDescriptions().emplace_back("-indent-arguments=align-paren",
        "Align arguments on continuing lines with the command's left parenthesis.");
    getCommandLineDescriptions().emplace_back(
        "-indent-arguments=STRING", "Use STRING for indenting arguments on continuing lines.");
    getCommandLineHandlers().emplace_back(&handleCommandLine);
}};
