/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_command_case(std::vector<Span> &spans, LetterCase letter_case) {

    size_t current_index = 0;
    while (current_index < spans.size()) {
        if (spans[current_index].type != SpanType::CommandIdentifier) {
            current_index++;
            continue;
        }
        if (letter_case == LetterCase::Lower) {
            spans[current_index].data = lowerstring(spans[current_index].data);
        } else if (letter_case == LetterCase::Upper) {
            spans[current_index].data = upperstring(spans[current_index].data);
        }
        current_index++;
    }
}

TEST_CASE("Makes command invocations lowercase") {
    REQUIRE_TRANSFORMS_TO(
        R"(
UPPERCASE_COMMAND()
mIxEdCaSe_CoMmAnD()
)",
        R"(
uppercase_command()
mixedcase_command()
)",
        transform_command_case, LetterCase::Lower);
}

TEST_CASE("Makes command invocations uppercase") {
    REQUIRE_TRANSFORMS_TO(
        R"(
lowercase_command()
mIxEdCaSe_CoMmAnD()
)",
        R"(
LOWERCASE_COMMAND()
MIXEDCASE_COMMAND()
)",
        transform_command_case, LetterCase::Upper);
}
