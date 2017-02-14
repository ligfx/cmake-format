/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <functional>

#include "formatting_indent.h"
#include "helpers.h"

using namespace std::placeholders;

std::vector<std::pair<std::string, std::string>> FormatterIndent::describeCommandLine() {
	return {{"-indent=STRING", "Use STRING for indentation."}};
}

bool FormatterIndent::handleCommandLine(const std::string &arg,
                                        std::vector<FormatterFunction> &formatter_functions) {
	if (arg.find("-indent=") != 0) {
		return false;
	}

	std::string indent_string = arg.substr(std::string{"-indent="}.size());
	replace_all_in_string(indent_string, "\\t", "\t");
	formatter_functions.emplace_back(std::bind(run, _1, _2, indent_string));

	return true;
};

void FormatterIndent::run(const std::vector<Command> &commands, std::vector<Span> &spans,
                          const std::string &indent_string) {
	int global_indentation_level = 0;
	for (auto c : commands) {
		const std::string ident = lowerstring(spans[c.identifier].data);

		if (ident == "endif" || ident == "endforeach" || ident == "endwhile" ||
		    ident == "endmacro" || ident == "endfunction") {
			global_indentation_level--;
		}

		auto indentation_level = global_indentation_level;
		if (ident == "else" || ident == "elseif") {
			indentation_level--;
		}

		const std::string old_indentation = spans[c.identifier - 1].data;

		// Re-indent the command invocation
		spans[c.identifier - 1].data = repeat_string(indent_string, indentation_level);

		// Walk forwards to fix arguments and the closing paren.
		size_t next_token = c.identifier + 1;
		while (true) {
			if (spans[next_token].type == SpanType::Newline) {
				size_t old_indentation_pos = spans[next_token + 1].data.find(old_indentation);
				if (old_indentation_pos != 0) {
				} else {
					spans[next_token + 1].data =
					    (repeat_string(indent_string, indentation_level) +
					     spans[next_token + 1].data.substr(old_indentation.size()));
				}
				next_token++;
			} else if (spans[next_token].type == SpanType::Rparen) {
				break;
			} else {
				next_token++;
			}
		}

		// Walk backwards to fix comments at the same prior indentation
		// level.
		// TODO: use iterators instead of fragile integer indices?
		std::ptrdiff_t last_token_on_previous_line = c.identifier - 3;
		while (last_token_on_previous_line >= 0) {
			if (spans[last_token_on_previous_line].type == SpanType::Comment &&
			    spans[last_token_on_previous_line - 1].type == SpanType::Space &&
			    spans[last_token_on_previous_line - 2].type == SpanType::Newline) {
				spans[last_token_on_previous_line - 1].data =
				    repeat_string(indent_string, indentation_level);
				last_token_on_previous_line -= 3;
			} else if (spans[last_token_on_previous_line].type == SpanType::Space &&
			           spans[last_token_on_previous_line - 1].type == SpanType::Newline) {
				last_token_on_previous_line -= 2;
			} else if (spans[last_token_on_previous_line].type == SpanType::Newline) {
				last_token_on_previous_line -= 1;
			} else {
				break;
			}
		}

		if (ident == "if" || ident == "foreach" || ident == "while" || ident == "macro" ||
		    ident == "function") {
			global_indentation_level++;
		}
	}
}
