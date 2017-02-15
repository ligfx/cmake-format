/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <algorithm>
#include <cctype>
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
#include "transform_argument_per_line.h"
#include "transform_indent.h"
#include "transform_lowercase_commands.h"

using namespace std::placeholders;

class TransformIndentArgument : public Transform {
	std::vector<std::pair<std::string, std::string>> describeCommandLine() override {
		return {
		    {"-indent-arguments=align-paren",
		     "Align arguments on continuing lines with the command's left parenthesis."},
		    {"-indent-arguments=STRING", "Use STRING for indenting arguments on continuing lines."},
		};
	}

	bool handleCommandLine(const std::string &arg,
	                       std::vector<TransformFunction> &tranform_functions) override {

		if (arg == "-indent-arguments=align-paren") {
			tranform_functions.emplace_back(std::bind(run, _1, _2, true, ""));
			return true;
		}
		if (arg.find("-indent-arguments=") == 0) {
			std::string argument_indent_string =
			    arg.substr(std::string{"-indent-arguments="}.size());
			replace_all_in_string(argument_indent_string, "\\t", "\t");
			tranform_functions.emplace_back(std::bind(run, _1, _2, false, argument_indent_string));
			return true;
		}

		return false;
	}

	static void run(const std::vector<Command> &commands, std::vector<Span> &spans,
	                bool argument_indent_after_lparen, std::string argument_indent_string) {

		(void)argument_indent_after_lparen;

		for (auto c : commands) {
			const std::string ident = lowerstring(spans[c.identifier].data);

			std::string command_indentation;
			if (spans[c.identifier - 1].type == SpanType::Newline) {
				command_indentation = "";
			} else {
				command_indentation = spans[c.identifier - 1].data;
			}

			if (argument_indent_after_lparen) {
				argument_indent_string = repeat_string(" ", ident.size() + 1);
			}

			// Walk forwards to fix argument indents.
			size_t next_token = c.identifier + 1;
			while (true) {
				if (spans[next_token].type == SpanType::Newline) {
					if (spans[next_token + 2].type != SpanType::Rparen) {
						spans[next_token + 1].data = command_indentation + argument_indent_string;
					}
					next_token++;
				} else if (spans[next_token].type == SpanType::Rparen) {
					break;
				} else {
					next_token++;
				}
			}
		}
	}
};

class TransformIndentRparen : public Transform {
	std::vector<std::pair<std::string, std::string>> describeCommandLine() override {
		return {{"-indent-rparen=STRING", "Use STRING for indenting hanging right-parens."}};
	}

	bool handleCommandLine(const std::string &arg,
	                       std::vector<TransformFunction> &tranform_functions) override {
		if (arg.find("-indent-rparen=") != 0) {
			return false;
		}

		std::string rparen_indent_string = arg.substr(std::string{"-indent-rparen="}.size());
		replace_all_in_string(rparen_indent_string, "\\t", "\t");

		tranform_functions.emplace_back(std::bind(run, _1, _2, rparen_indent_string));

		return true;
	}

	static void run(const std::vector<Command> &commands, std::vector<Span> &spans,
	                const std::string &rparen_indent_string) {
		for (auto c : commands) {
			const std::string ident = lowerstring(spans[c.identifier].data);

			std::string command_indentation;
			if (spans[c.identifier - 1].type == SpanType::Newline) {
				command_indentation = "";
			} else {
				command_indentation = spans[c.identifier - 1].data;
			}

			// Walk forwards to fix continuation indents.
			size_t next_token = c.identifier + 1;
			while (true) {
				if (spans[next_token].type == SpanType::Rparen) {
					if (spans[next_token - 2].type == SpanType::Newline) {
						spans[next_token - 1].data = (command_indentation + rparen_indent_string);
					}
					break;
				} else {
					next_token++;
				}
			}
		}
	}
};

int main(int argc, char **argv) {

	std::vector<std::unique_ptr<Transform>> formatters;
	formatters.emplace_back(new TransformArgumentPerLine());
	formatters.emplace_back(new TransformLowercaseCommands());
	formatters.emplace_back(new TransformIndent());
	formatters.emplace_back(new TransformIndentArgument());
	formatters.emplace_back(new TransformIndentRparen());

	bool format_in_place = false;
	std::vector<TransformFunction> formatting_functions;

	std::vector<std::string> filenames;
	for (int i = 1; i < argc; i++) {
		const std::string arg{argv[i]};
		if (arg[0] == '-') {
			std::smatch reindent_match;

			if ("-help" == arg || "-h" == arg || "--help" == arg) {
				fprintf(stderr,
				        "usage: %s [options] [file ...]\n"
				        "\n"
				        "Re-formats specified files. If -i is specified, formats files\n"
				        "in-place; otherwise, writes results to standard output.\n"
				        "\n",
				        argv[0]);
				fprintf(stderr, "options:\n");

				size_t max_option_size = 0;
				for (auto const &f : formatters) {
					for (auto const p : f->describeCommandLine()) {
						max_option_size = std::max(max_option_size, p.first.size());
					}
				}
				for (auto const &f : formatters) {
					for (auto const p : f->describeCommandLine()) {
						std::string opt;
						std::string description;
						std::tie(opt, description) = p;
						fprintf(stderr, "  %s%s  %s\n", opt.c_str(),
						        repeat_string(" ", max_option_size - opt.size()).c_str(),
						        description.c_str());
					}
				}
				fprintf(stderr, "  %s%s  %s\n", "-i",
				        repeat_string(" ", max_option_size - 2).c_str(),
				        "Re-format files in-place.");
				exit(1);
			} else if (arg == "-i") {
				format_in_place = true;
			} else {
				bool handled_arg = false;
				for (const auto &f : formatters) {
					if ((handled_arg = f->handleCommandLine(arg, formatting_functions))) {
						break;
					}
				}
				if (!handled_arg) {
					fprintf(stderr, "%s: unrecognized option '%s'. Try: %s -help\n", argv[0],
					        arg.c_str(), argv[0]);
					exit(1);
				}
			}
		} else {
			filenames.emplace_back(arg);
		}
	}

	if (filenames.size() == 0) {
		fprintf(stderr, "%s: no filenames specified. Try: %s -help\n", argv[0], argv[0]);
		exit(1);
	}

	if (formatting_functions.size() == 0) {
		fprintf(stderr, "%s: no formatting options specified. Try: %s -help\n", argv[0], argv[0]);
		exit(1);
	}

	for (auto filename : filenames) {
		std::string content;
		{
			std::ifstream file{filename};
			content = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
		}

		std::vector<Span> spans;
		std::vector<Command> commands;
		std::tie(spans, commands) = parse(content);

		for (auto f : formatting_functions) {
			f(commands, spans);
		}

		if (format_in_place) {
			std::ofstream file{filename};
			for (auto s : spans) {
				file << s.data;
			}
		} else {
			for (auto s : spans) {
				std::cout << s.data;
			}
		}
	}
}
