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

int main(int argc, char **argv) {

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
				for (auto const &p : getCommandLineDescriptions()) {
					max_option_size = std::max(max_option_size, p.first.size());
				}

				for (auto const &p : getCommandLineDescriptions()) {
					std::string opt;
					std::string description;
					std::tie(opt, description) = p;
					fprintf(stderr, "  %s%s  %s\n", opt.c_str(),
					        repeat_string(" ", max_option_size - opt.size()).c_str(),
					        description.c_str());
				}

				fprintf(stderr, "  %s%s  %s\n", "-i",
				        repeat_string(" ", max_option_size - 2).c_str(),
				        "Re-format files in-place.");
				exit(1);
			} else if (arg == "-i") {
				format_in_place = true;
			} else {
				bool handled_arg = false;
				for (const auto &f : getCommandLineHandlers()) {
					if ((handled_arg = f(arg, formatting_functions))) {
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
