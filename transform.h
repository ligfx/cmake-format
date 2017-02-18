/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <string>
#include <vector>

#include "helpers.h"
#include "parser.h"

void transform_argument_bin_pack(std::vector<Span> &, size_t, const std::string &);
void transform_argument_heuristic(std::vector<Span> &, size_t, const std::string &);
void transform_argument_per_line(std::vector<Span> &, const std::string &);
void transform_command_case(std::vector<Span> &, LetterCase);
void transform_indent(std::vector<Span> &, const std::string &);
void transform_indent_rparen(std::vector<Span> &, const std::string &);
void transform_loosen_loop_constructs(std::vector<Span> &);
void transform_space_before_parens(std::vector<Span> &, SpaceBeforeParens);
void transform_squash_empty_lines(std::vector<Span> &, size_t);
