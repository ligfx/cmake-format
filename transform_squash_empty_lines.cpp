/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "transform.h"

void transform_squash_empty_lines(std::vector<Span> &spans, size_t max_empty_lines) {

    size_t current_index = 0;
    size_t preceding_newlines = 0;
    while (current_index < spans.size()) {
        if (current_index + 1 < spans.size() && spans[current_index].type == SpanType::Space &&
            spans[current_index + 1].type == SpanType::Newline) {
            // delete trailing space
            delete_span(spans, current_index);
        }
        if (spans[current_index].type == SpanType::Newline) {
            if (preceding_newlines >= max_empty_lines + 1) {
                delete_span(spans, current_index);
            } else {
                current_index++;
            }
            preceding_newlines += 1;
            continue;
        }
        preceding_newlines = 0;
        current_index++;
    }
}

TEST_CASE("Squashes empty lines") {
    REQUIRE_TRANSFORMS_TO(
        R"(
command()

command()



command()




command()
)",
        R"(
command()

command()

command()

command()
)",
        transform_squash_empty_lines, 1);
}
