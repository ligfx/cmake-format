cmake_minimum_required(VERSION 3.0)
project(cmake-format LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(-fcolor-diagnostics)
endif()

add_executable(cmake-format
    cmake-format.cpp
    parser.cpp
)
target_compile_options(cmake-format PRIVATE -Wall -Wextra -pedantic -Werror -Wno-error=unused-variable)
set_target_properties(cmake-format PROPERTIES CXX_STANDARD 11)
