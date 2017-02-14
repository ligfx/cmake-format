/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <catch.hpp>

#include "parser.h"

TEST_CASE("Parses CMake code", "[parsing]") {
	(void)parse(R"(

cmake_command_without_arguments()
_underscore_cmake_command()
UPPERCASE_CMAKE_COMMAND()
c3e_c5d_w2h_n5s()
simple_cmake_command(
	simple_argument
	UPPERCASE_ARGUMENT
	_argument_starting_with_underscore
	-argument-with-dashes
	a6t_w2h_n5s
	1234567890
	argument;with;semicolons
	argument/with/slashes
	argument\\with\\backslashes
	argument_with_\(parentheses\)
	"quoted argument"
	bare_argument_with_"quoted argument"
	# comment
	${bare_variable_reference}
	${make_style_base_variable_reference}
	${variable_reference_${embedded_variable_reference}}
	$<$<generator_expression>:value can be anything! ${variable_reference}>
)

)");
}

#ifndef _MSC_VER
// HACK: this doesn't work on Windows
// TODO: what exception has message "basic_string"?
TEST_CASE("Doesn't hang on unbalanced parentheses", "[parsing]") {
	REQUIRE_THROWS(parse(R"(command()"));
}
#endif

TEST_CASE("Parses bare parentheses in arguments", "[parsing]") {
	(void)parse(R"(simple_cmake_command(()))");
}
