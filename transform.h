/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "parser.h"

using TransformFunction = std::function<void(std::vector<Command> &, std::vector<Span> &)>;
using HandleCommandLineFunction = std::function<bool(
    const std::string &arg, std::vector<TransformFunction> &transform_functions)>;

std::vector<std::pair<std::string, std::string>> &getCommandLineDescriptions();
std::vector<HandleCommandLineFunction> &getCommandLineHandlers();
