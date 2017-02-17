cmake_minimum_required(VERSION 3.0)
project(cmake-format)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "CMAKE_BUILD_TYPE not specified, defaulting to 'Debug'.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

if(UNIX AND EXISTS ${PROJECT_SOURCE_DIR}/.git AND NOT EXISTS ${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit)
    message(STATUS "Linking lint.sh -> .git/hooks/pre-commit")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E create_symlink ../../lint.sh pre-commit
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/.git/hooks
    )
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
    add_compile_options(-Wall -Wextra -pedantic -Werror -Wshadow)
endif()

include_directories(
    3rdparty/catch-1.7.2
    3rdparty/cmake-3.7.2/Source
    3rdparty/akrzemi1-Optional-25713110f
)

set(CMAKEFORMAT_SOURCES
    transform_argument_bin_pack.cpp
    transform_argument_heuristic.cpp
    transform_argument_per_line.cpp
    transform_indent.cpp
    transform_indent_rparen.cpp
    transform_lowercase_commands.cpp
    parser.cpp
    generated/cmListFileLexer.c
)
set_source_files_properties(generated/cmListFileLexer.c PROPERTIES COMPILE_FLAGS -w)

add_executable(cmake-format
    cmake-format.cpp
    ${CMAKEFORMAT_SOURCES}
)

add_executable(cmake-format-tests EXCLUDE_FROM_ALL
    cmake-format-tests.cpp
    ${CMAKEFORMAT_SOURCES}
)
target_compile_definitions(cmake-format-tests PUBLIC CMAKEFORMAT_USE_CATCH)
add_custom_target(check COMMAND $<TARGET_FILE:cmake-format-tests> --use-colour yes)
