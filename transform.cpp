/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include "transform.h"

std::vector<std::pair<std::string, std::string>> &getCommandLineDescriptions() {
	static std::vector<std::pair<std::string, std::string>> command_line_descriptions;
	return command_line_descriptions;
}

std::vector<HandleCommandLineFunction> &getCommandLineHandlers() {
	static std::vector<HandleCommandLineFunction> command_line_handlers;
	return command_line_handlers;
}
