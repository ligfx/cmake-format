/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "transform.h"

void transform_loosen_loop_constructs(std::vector<Span> &spans) {

    size_t current_index = 0;
    while (current_index < spans.size()) {
        if (spans[current_index].type != SpanType::CommandIdentifier) {
            current_index++;
            continue;
        }
        const std::string ident = lowerstring(spans[current_index].data);
        current_index++;

        if (!(ident == "else" || ident == "endif" || ident == "endwhile" || ident == "endmacro" ||
                ident == "endfunction" || ident == "endforeach")) {
            continue;
        }

        while (spans[current_index].type != SpanType::Lparen) {
            current_index++;
        }
        current_index++;
        while (spans[current_index].type != SpanType::Rparen) {
            if (spans[current_index].type == SpanType::Comment) {
                current_index += 2; // skip newline
                continue;
            }
            delete_span(spans, current_index);
        }

        current_index++;
    }
}

TEST_CASE("Loosens strict loop constructs") {
    REQUIRE_TRANSFORMS_TO(
        R"(
if(HELLO)
command(ARGS)
else(
    HELLO )
endif(HELLO # comment
)
)",
        R"(
if(HELLO)
command(ARGS)
else()
endif(# comment
)
)",
        transform_loosen_loop_constructs);
}
