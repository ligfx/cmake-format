/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <string>
#include <vector>

#include "parser.h"

void transform_argument_bin_pack(std::vector<Command> &commands, std::vector<Span> &spans,
    size_t column_limit, const std::string &argument_indent_string);
void transform_argument_heuristic(std::vector<Command> &commands, std::vector<Span> &spans,
    size_t column_limit, const std::string &argument_indent_string);
void transform_argument_per_line(std::vector<Command> &commands, std::vector<Span> &spans,
    const std::string &argument_indent_string);
void transform_indent(
    std::vector<Command> &commands, std::vector<Span> &spans, const std::string &indent_string);
void transform_indent_rparen(std::vector<Command> &commands, std::vector<Span> &spans,
    const std::string &rparen_indent_string);
void transform_lowercase_commands(std::vector<Command> &commands, std::vector<Span> &spans);
