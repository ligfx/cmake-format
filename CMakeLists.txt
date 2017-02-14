cmake_minimum_required(VERSION 3.0)
project(cmake-format LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(-fcolor-diagnostics)
endif()

add_library(libcmakeformat OBJECT
    parser.cpp
)

add_executable(cmake-format
    cmake-format.cpp
    $<TARGET_OBJECTS:libcmakeformat>
)
target_compile_options(cmake-format PRIVATE -Wall -Wextra -pedantic -Werror -Wno-error=unused-variable)

add_executable(cmake-format-tests EXCLUDE_FROM_ALL
    test_main.cpp
    test_parsing.cpp
    $<TARGET_OBJECTS:libcmakeformat>
)
target_include_directories(cmake-format-tests PRIVATE 3rdparty/catch-1.7.2)
add_custom_target(check COMMAND $<TARGET_FILE:cmake-format-tests> --use-colour yes)
