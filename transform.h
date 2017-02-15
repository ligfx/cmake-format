/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "parser.h"

using TransformFunction = std::function<void(const std::vector<Command> &, std::vector<Span> &)>;

struct Transform {
	virtual ~Transform() = default;
	virtual std::vector<std::pair<std::string, std::string>> describeCommandLine() = 0;
	virtual bool handleCommandLine(const std::string &arg,
	                               std::vector<TransformFunction> &transform_functions) = 0;
};