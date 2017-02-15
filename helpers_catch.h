/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <catch.hpp>

#include "helpers.h"
#include "parser.h"
#include "transform.h"

static inline void REQUIRE_TRANSFORMS_TO(TransformFunction transform, std::string original,
                                         std::string wanted) {
	std::vector<Span> spans;
	std::vector<Command> commands;
	std::tie(spans, commands) = parse(original);

	transform(commands, spans);
	std::string output;
	for (auto s : spans) {
		output += s.data;
	}

	replace_invisibles_with_visibles(output);
	replace_invisibles_with_visibles(wanted);

	REQUIRE(output == wanted);
}
