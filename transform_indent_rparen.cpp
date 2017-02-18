/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_indent_rparen(std::vector<Span> &spans, const std::string &rparen_indent_string) {

    size_t next_token = 0;
    while (next_token < spans.size()) {
        if (spans[next_token].type != SpanType::CommandIdentifier) {
            next_token++;
            continue;
        }

        const size_t identifier_index = next_token;
        const std::string ident = lowerstring(spans[identifier_index].data);

        std::string command_indentation;
        if (spans[identifier_index - 1].type == SpanType::Newline) {
            command_indentation = "";
        } else {
            command_indentation = spans[identifier_index - 1].data;
        }

        // Walk forwards to fix continuation indents.
        next_token++;
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
        next_token++;
    }
}
