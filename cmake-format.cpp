/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "command_line.h"
#include "helpers.h"
#include "parser.h"
#include "transform.h"

template <typename T, typename U>
void parse_and_transform_and_write(
    T &&file_in, U &&file_out, const std::vector<TransformFunction> &transform_functions) {
    std::string content;
    { content = {std::istreambuf_iterator<char>(file_in), std::istreambuf_iterator<char>()}; }

    std::vector<Span> spans;
    std::vector<Command> commands;
    std::tie(spans, commands) = parse(content);

    for (auto f : transform_functions) {
        f(commands, spans);
    }
    for (auto s : spans) {
        file_out << s.data;
    }
}

enum class LetterCase {
    Lower,
    Upper,
};

int main(int argc, char **argv) {
    struct {
        LetterCase command_case{LetterCase::Lower};
        size_t indent_width{4};
    } config;
    bool format_in_place = false;

    const static std::string description =
        "Re-formats specified files. If no files are specified on the command-line,\n"
        "reads from standard input. If -i is specified, formats files in-place;\n"
        "otherwise, writes results to standard output.";

    const static std::vector<SwitchOptionDescription> switch_options = {
        {"-i", "Re-format files in-place.", format_in_place},
    };

    const static std::vector<ArgumentOptionDescription> argument_options = {
        {"-command-case", "\"lower\"|\"upper\"", "Letter case of command invocations.",
            [&](const std::string &value) {
                if (value == "lower") {
                    config.command_case = LetterCase::Lower;
                } else if (value == "upper") {
                    config.command_case = LetterCase::Upper;
                } else {
                    throw opterror;
                }
            }},
        {"-indent-width", "NUMBER", "Use NUMBER spaces for indentation.",
            [&](const std::string &value) {
                try {
                    config.indent_width = std::stoi(value);
                } catch (const std::invalid_argument &) {
                    throw opterror;
                }
            }},
        // {"-indent-rparen=STRING", "Use STRING for indenting hanging right-parens."},
        // {"-argument-per-line=STRING", "Put each argument on its own line, indented by STRING."},
        // {"-argument-bin-pack=WIDTH", "\"Bin pack\" arguments, with a maximum column width of
        // WIDTH."},
    };

    std::vector<std::string> filenames =
        parse_command_line(argc, argv, description, switch_options, argument_options);

    std::vector<TransformFunction> transform_functions;
    transform_functions.emplace_back(
        std::bind(transform_indent, _1, _2, repeat_string(" ", config.indent_width)));
    if (config.command_case == LetterCase::Lower) {
        transform_functions.emplace_back(&transform_lowercase_commands);
    } else {
        fprintf(stderr, "%s: -command-case=upper is not implemented\n", argv[0]);
        exit(1);
    }

    if (filenames.size() == 0) {
        if (format_in_place) {
            fprintf(stderr, "%s: '-i' specified without any filenames. Try: %s -help\n", argv[0],
                argv[0]);
            exit(1);
        }
        parse_and_transform_and_write(std::cin, std::cout, transform_functions);
    }

    for (auto filename : filenames) {
        std::ifstream file_in{filename};
        if (format_in_place) {
            parse_and_transform_and_write(file_in, std::ofstream{filename}, transform_functions);
        } else {
            parse_and_transform_and_write(file_in, std::cout, transform_functions);
        }
    }
}
