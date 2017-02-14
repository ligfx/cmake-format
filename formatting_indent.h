/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include "formatting.h"

struct FormatterIndent : public Formatter {
	std::vector<std::pair<std::string, std::string>> describeCommandLine() override;

	bool handleCommandLine(const std::string &arg,
	                       std::vector<FormatterFunction> &formatter_functions) override;

	static void run(const std::vector<Command> &commands, std::vector<Span> &spans,
	                const std::string &indent_string);
};
