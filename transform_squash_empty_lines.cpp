/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "transform.h"

void transform_squash_empty_lines(
    std::vector<Command> &commands, std::vector<Span> &spans, size_t max_empty_lines) {
    size_t next_token = 0;
    size_t preceding_newlines = 0;
    while (next_token < spans.size()) {
        if (next_token + 1 < spans.size() && spans[next_token].type == SpanType::Space &&
            spans[next_token + 1].type == SpanType::Newline) {
            // delete trailing space
            delete_span(commands, spans, next_token);
        }
        if (spans[next_token].type == SpanType::Newline) {
            if (preceding_newlines >= max_empty_lines + 1) {
                delete_span(commands, spans, next_token);
            } else {
                next_token++;
            }
            preceding_newlines += 1;
            continue;
        }
        preceding_newlines = 0;
        next_token++;
    }
}

TEST_CASE("Squashes empty lines", "[transform.squash_empty_lines]") {
    REQUIRE_TRANSFORMS_TO(std::bind(transform_squash_empty_lines, _1, _2, 1),
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
)");
}
