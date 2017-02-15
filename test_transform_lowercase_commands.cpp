/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <catch.hpp>

#include "helpers_catch.h"
#include "transform_lowercase_commands.h"

TEST_CASE("Makes command invocations lowercase", "[transform.lowercase_commands]") {
	REQUIRE_TRANSFORMS_TO(TransformLowercaseCommands::run,
	                      R"(
UPPERCASE_COMMAND()
mIxEdCaSe_CoMmAnD()
)",
	                      R"(
uppercase_command()
mixedcase_command()
)");
}
