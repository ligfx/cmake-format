/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"

static void run(const std::vector<Command> &commands, std::vector<Span> &spans) {
    for (auto c : commands) {
        spans[c.identifier].data = lowerstring(spans[c.identifier].data);
    }
}

static bool handleCommandLine(
    const std::string &arg, std::vector<TransformFunction> &formatting_functions) {
    if (arg != "-lowercase-commands")
        return false;
    formatting_functions.emplace_back(&run);
    return true;
}

static const on_program_load transform_argument_per_line{[]() {
    getCommandLineDescriptions().emplace_back(
        "-lowercase-commands", "Lowercase command names in command invocations.");
    getCommandLineHandlers().emplace_back(&handleCommandLine);
}};

TEST_CASE("Makes command invocations lowercase", "[transform.lowercase_commands]") {
    REQUIRE_TRANSFORMS_TO(run,
        R"(
UPPERCASE_COMMAND()
mIxEdCaSe_CoMmAnD()
)",
        R"(
uppercase_command()
mixedcase_command()
)");
}
