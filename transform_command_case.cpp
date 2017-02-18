/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_command_case(
    std::vector<Command> &commands, std::vector<Span> &spans, LetterCase letter_case) {
    for (auto c : commands) {
        if (letter_case == LetterCase::Lower) {
            spans[c.identifier].data = lowerstring(spans[c.identifier].data);
        } else if (letter_case == LetterCase::Upper) {
            spans[c.identifier].data = upperstring(spans[c.identifier].data);
        }
    }
}

TEST_CASE("Makes command invocations lowercase") {
    REQUIRE_TRANSFORMS_TO(std::bind(transform_command_case, _1, _2, LetterCase::Lower),
        R"(
UPPERCASE_COMMAND()
mIxEdCaSe_CoMmAnD()
)",
        R"(
uppercase_command()
mixedcase_command()
)");
}

TEST_CASE("Makes command invocations uppercase") {
    REQUIRE_TRANSFORMS_TO(std::bind(transform_command_case, _1, _2, LetterCase::Upper),
        R"(
lowercase_command()
mIxEdCaSe_CoMmAnD()
)",
        R"(
LOWERCASE_COMMAND()
MIXEDCASE_COMMAND()
)");
}
