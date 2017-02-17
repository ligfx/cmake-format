/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform.h"

void transform_lowercase_commands(std::vector<Command> &commands, std::vector<Span> &spans) {
    for (auto c : commands) {
        spans[c.identifier].data = lowerstring(spans[c.identifier].data);
    }
}

TEST_CASE("Makes command invocations lowercase", "[transform.lowercase_commands]") {
    REQUIRE_TRANSFORMS_TO(transform_lowercase_commands,
        R"(
UPPERCASE_COMMAND()
mIxEdCaSe_CoMmAnD()
)",
        R"(
uppercase_command()
mixedcase_command()
)");
}
