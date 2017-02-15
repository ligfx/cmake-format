/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <catch.hpp>

#include "helpers.h"
#include "parser.h"
#include "transform_indent.h"

using namespace std::placeholders;

void REQUIRE_TRANSFORMS_TO(TransformFunction transform, std::string original, std::string wanted) {
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

TEST_CASE("Reindents toplevel", "[transform.indent]") {
	REQUIRE_TRANSFORMS_TO(std::bind(TransformIndent::run, _1, _2, "INDENT "),
	                      R"(
   improperly_indented_toplevel()
correctly_indented_toplevel()
)",
	                      R"(
improperly_indented_toplevel()
correctly_indented_toplevel()
)");
}

TEST_CASE("Reindents arguments", "[transform.indent]") {
	REQUIRE_TRANSFORMS_TO(std::bind(TransformIndent::run, _1, _2, "INDENT "),
	                      R"(
    command(
       ARGUMENT
           ANOTHER_ARGUMENT
    )
if()
command(
ARGUMENT
)
endif()
)",
	                      R"(
command(
   ARGUMENT
       ANOTHER_ARGUMENT
)
if()
INDENT command(
INDENT ARGUMENT
INDENT )
endif()
)");
}

TEST_CASE("Reindents comments", "[transform.indent]") {
	REQUIRE_TRANSFORMS_TO(std::bind(TransformIndent::run, _1, _2, "INDENT "),
	                      R"(
    # associated comment
    # with multiple lines
    command()
)",
	                      R"(
# associated comment
# with multiple lines
command()
)");
}

TEST_CASE("Reindents blocks", "[transform.indent]") {
	REQUIRE_TRANSFORMS_TO(std::bind(TransformIndent::run, _1, _2, "INDENT "),
	                      R"(
   if(CONDITION)
command()
       if(ANOTHER_CONDITION)
command()
endif()
           elseif(YET_ANOTHER_CONDITION)
  else(SHOULD_NOT_BE_INDENTED)
command()
endif()

       foreach()
command()
endforeach()

   macro()
command()
   endmacro()

while()
command()
endwhile()
)",
	                      R"(
if(CONDITION)
INDENT command()
INDENT if(ANOTHER_CONDITION)
INDENT INDENT command()
INDENT endif()
elseif(YET_ANOTHER_CONDITION)
else(SHOULD_NOT_BE_INDENTED)
INDENT command()
endif()

foreach()
INDENT command()
endforeach()

macro()
INDENT command()
endmacro()

while()
INDENT command()
endwhile()
)");
}
