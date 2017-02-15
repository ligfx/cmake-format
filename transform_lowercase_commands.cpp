/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "helpers.h"
#include "transform_lowercase_commands.h"

std::vector<std::pair<std::string, std::string>> TransformLowercaseCommands::describeCommandLine() {
	return {{"-lowercase-commands", "Lowercase command names in command invocations."}};
}

bool TransformLowercaseCommands::handleCommandLine(
    const std::string &arg, std::vector<TransformFunction> &formatting_functions) {
	if (arg != "-lowercase-commands")
		return false;
	formatting_functions.emplace_back(&run);
	return true;
}

void TransformLowercaseCommands::run(const std::vector<Command> &commands,
                                     std::vector<Span> &spans) {
	for (auto c : commands) {
		spans[c.identifier].data = lowerstring(spans[c.identifier].data);
	}
}
