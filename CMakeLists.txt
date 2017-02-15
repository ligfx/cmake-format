cmake_minimum_required(VERSION 3.0)
project(cmake-format)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "CMAKE_BUILD_TYPE not specified, defaulting to 'Debug'.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 11)
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(-fcolor-diagnostics $<$<CONFIG:Debug>:-fsanitize=address>)
    link_libraries($<$<CONFIG:Debug>:-fsanitize=address>)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /WX /EHsc
        /wd4706  # assignment within conditional expression
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_library(libcmakeformat OBJECT
    transform_indent.cpp
    transform_lowercase_commands.cpp
    parser.cpp
    generated/cmListFileLexer.c
)
set_source_files_properties(generated/cmListFileLexer.c PROPERTIES COMPILE_FLAGS -w)
target_include_directories(libcmakeformat PRIVATE 3rdparty/cmake-3.7.2/Source)

add_executable(cmake-format
    cmake-format.cpp
    $<TARGET_OBJECTS:libcmakeformat>
)

add_executable(cmake-format-tests EXCLUDE_FROM_ALL
    test_main.cpp
    test_transform_indent.cpp
    test_parsing.cpp
    $<TARGET_OBJECTS:libcmakeformat>
)
target_include_directories(cmake-format-tests PRIVATE 3rdparty/catch-1.7.2)
add_custom_target(check COMMAND $<TARGET_FILE:cmake-format-tests> --use-colour yes)
