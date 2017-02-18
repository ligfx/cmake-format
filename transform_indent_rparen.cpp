/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_indent_rparen(std::vector<Span> &spans, const std::string &rparen_indent_string) {

    size_t current_index = 0;
    while (current_index < spans.size()) {
        if (spans[current_index].type != SpanType::CommandIdentifier) {
            current_index++;
            continue;
        }

        const size_t identifier_index = current_index;
        const std::string ident = lowerstring(spans[identifier_index].data);

        std::string command_indentation;
        if (spans[identifier_index - 1].type == SpanType::Newline) {
            command_indentation = "";
        } else {
            command_indentation = spans[identifier_index - 1].data;
        }

        // Walk forwards to fix continuation indents.
        current_index++;
        while (true) {
            if (spans[current_index].type == SpanType::Rparen) {
                if (spans[current_index - 2].type == SpanType::Newline) {
                    spans[current_index - 1].data = (command_indentation + rparen_indent_string);
                }
                break;
            } else {
                current_index++;
            }
        }
        current_index++;
    }
}
