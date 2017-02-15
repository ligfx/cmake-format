/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <catch.hpp>

#include "helpers_catch.h"
#include "transform_argument_per_line.h"

using namespace std::placeholders;

TEST_CASE("Puts each argument on its own line", "[transform.argument_per_line]") {
	REQUIRE_TRANSFORMS_TO(std::bind(TransformArgumentPerLine::run, _1, _2, "INDENT "),
	                      R"(
    command(ARG1 ARG2 ARG3)
    cmake_minimum_required(VERSION 3.0)
    project(cmake-format)
)",
	                      R"(
    command(
    INDENT ARG1
    INDENT ARG2
    INDENT ARG3
    )
    cmake_minimum_required(
    INDENT VERSION
    INDENT 3.0
    )
    project(
    INDENT cmake-format
    )
)");
}
