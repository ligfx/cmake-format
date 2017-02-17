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

static std::vector<std::pair<std::string, std::string>> command_line_descriptions = {
    {"-command-case=lower|upper", "Letter case of command invocations."},
    {"-indent-width=NUMBER", "Use NUMBER spaces for indentation."},
    // {"-indent-rparen=STRING", "Use STRING for indenting hanging right-parens."},
    // {"-argument-per-line=STRING", "Put each argument on its own line, indented by STRING."},
    // {"-argument-bin-pack=WIDTH", "\"Bin pack\" arguments, with a maximum column width of
    // WIDTH."},
};

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
    std::vector<TransformFunction> transform_functions;

    std::vector<std::string> filenames;
    for (int i = 1; i < argc; i++) {
        const std::string arg{argv[i]};
        if (arg[0] == '-') {
            std::smatch reindent_match;

            if ("-help" == arg || "-h" == arg || "--help" == arg) {
                fprintf(stderr, R"(
usage: %s [options] [file ...]

Re-formats specified files. If no files are specified on the command-line,
reads from standard input. If -i is specified, formats files in-place;
otherwise, writes results to standard output.

options:
)",
                    argv[0]);

                size_t max_option_size = 0;
                for (auto const &p : command_line_descriptions) {
                    max_option_size = std::max(max_option_size, p.first.size());
                }

                for (auto const &p : command_line_descriptions) {
                    std::string opt;
                    std::string description;
                    std::tie(opt, description) = p;
                    fprintf(stderr, "  %s%s  %s\n", opt.c_str(),
                        repeat_string(" ", max_option_size - opt.size()).c_str(),
                        description.c_str());
                }

                fprintf(stderr, "  %s%s  %s\n", "-i",
                    repeat_string(" ", max_option_size - 2).c_str(), "Re-format files in-place.");
                exit(1);
            } else if (arg == "-i") {
                format_in_place = true;
            } else {
                if (arg == "-indent-width") {
                    fprintf(
                        stderr, "%s: for the -indent-width option: requires a value!\n", argv[0]);
                    exit(1);
                } else if (arg.find("-indent-width=") == 0) {
                    config.indent_width =
                        std::stoi(arg.substr(std::string{"-indent-width="}.size()));
                } else if (arg == "-command-case") {
                    fprintf(
                        stderr, "%s: for the -command-case option: requires a value!\n", argv[0]);
                    exit(1);
                } else if (arg.find("-command-case=") == 0) {
                    const auto command_case = arg.substr(std::string{"-command-case="}.size());
                    if (command_case == "lower") {
                        config.command_case = LetterCase::Lower;
                    } else if (command_case == "upper") {
                        config.command_case = LetterCase::Upper;
                    } else {
                        fprintf(stderr, "%s: for the -command-case option: '%s' value invalid!\n",
                            argv[0], command_case.c_str());
                        exit(1);
                    }
                } else {
                    fprintf(stderr, "%s: unrecognized option '%s'. Try: %s -help\n", argv[0],
                        arg.c_str(), argv[0]);
                    exit(1);
                }
            }
        } else {
            filenames.emplace_back(arg);
        }
    }

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
