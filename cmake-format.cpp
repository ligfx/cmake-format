/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <cassert>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <utility>
#include <vector>

using namespace std::placeholders;

static std::string content;
static size_t p = 0;

enum class SpanType {
	Identifier,
	Quoted,
	Unquoted,
	Newline,
	Comment,
	Space,
	Lparen,
	Rparen,
};

struct Span {
	Span(const SpanType &type_, const std::string &data_) : type(type_), data(data_) {
	}
	SpanType type;
	std::string data;
};

static std::vector<Span> spans;

void replace_all_in_string(std::string &main_string, const std::string &from,
                           const std::string &to) {
	size_t pos = 0;
	while ((pos = main_string.find(from, pos)) != std::string::npos) {
		main_string.replace(pos, from.size(), to);
	}
}

struct Command {
	Command(size_t identifer_, std::vector<size_t> arguments_)
	    : identifier(identifer_), arguments(arguments_) {
	}
	const size_t identifier;
	const std::vector<size_t> arguments;
};

bool isIdentifierStart(char c) {
	return std::isalpha(c) || c == '_';
}

bool isIdentifier(char c) {
	return std::isalnum(c) || c == '_';
}

void skip_ws() {
	while (true) {
		if (content[p] == '\n') {
			spans.emplace_back(SpanType::Newline, content.substr(p, 1));
			p++;
			if (!std::isspace(content[p])) {
				// HACK: Make sure newlines are always followed by whitespace, this makes
				// transformations easier.
				spans.emplace_back(SpanType::Space, "");
			}
			continue;
		}
		if (std::isspace(content[p])) {
			size_t space_start = p;
			while (std::isspace(content[p]) && content[p] != '\n') {
				p++;
			}
			spans.emplace_back(SpanType::Space, content.substr(space_start, p - space_start));
			continue;
		}
		if (content[p] == '#') {
			size_t comment_start = p;
			while (content[p] != '\n') {
				p++;
			}
			spans.emplace_back(SpanType::Comment, content.substr(comment_start, p - comment_start));
			continue;
		}
		break;
	}
}

void expect(bool condition, const std::string &description) {
	if (!condition) {
		printf("FAILURE: Expected %s at %zu, got:\n%s\n", description.c_str(), p,
		       content.substr(p, p + 50).c_str());
		exit(1);
	}
}

Span parse_identifier() {
	expect(isIdentifierStart(content[p]), "identifier start");
	size_t ident_start = p;
	p++;
	while (isIdentifier(content[p])) {
		p++;
	}
	return {SpanType::Identifier, content.substr(ident_start, p - ident_start)};
}

void parse_lparen() {
	expect(content[p] == '(', "left paren");
	spans.emplace_back(SpanType::Lparen, content.substr(p, 1));
	p++;
}

void parse_rparen() {
	expect(content[p] == ')', "right paren");
	if (spans.back().type != SpanType::Space) {
		// HACK: Make sure right-parens are always preceded by whitespace, this makes
		// transformations easier.
		spans.emplace_back(SpanType::Space, "");
	}
	spans.emplace_back(SpanType::Rparen, content.substr(p, 1));
	p++;
}

void parse_dollar_sign_expression() {
	expect(content[p] == '$', "dollar-sign");
	size_t variable_reference_start = p;
	p++;

	char endbracket;
	if (content[p] == '{') {
		p++;
		endbracket = '}';
	} else if (content[p] == '(') {
		p++;
		endbracket = ')';
	} else if (content[p] == '<') {
		p++;
		endbracket = '>';
	} else {
		expect(false, "'{', '(', or '<'");
	}

	while (true) {
		// HACK: this is more lenient than the real CMake parser
		if (content[p] == '$') {
			parse_dollar_sign_expression();
			continue;
		} else if (content[p] == endbracket) {
			p++;
			break;
		}
		p++;
	}
}

Span parse_quoted_argument() {
	expect(content[p] == '"', "double-quote");
	size_t quoted_argument_start = p;
	p++;
	while (true) {
		if (content[p] == '\\') {
			p++;
			p++;
		} else if (content[p] == '"') {
			p++;
			break;
		} else {
			p++;
		}
	}
	return {SpanType::Quoted, content.substr(quoted_argument_start, p - quoted_argument_start)};
}

bool is_unquoted_element(const char c) {
	// TODO: instead of std::isspace, also need to skip anything CMake thinks is
	// whitespace (like comments)
	return !(std::isspace(c) || c == '\\' || c == '$' || c == '"' || c == '(' || c == ')');
}

Span parse_unquoted_argument() {
	size_t unquoted_argument_start = p;
	while (true) {
		if (content[p] == '\\') {
			fprintf(stderr, "escape sequences unimplemented\n");
			exit(1);
		}
		if (content[p] == '$') {
			parse_dollar_sign_expression();
			continue;
		}
		if (content[p] == '"') {
			parse_quoted_argument();
			continue;
		}
		if (is_unquoted_element(content[p])) {
			size_t unquoted_element_start = p;
			while (is_unquoted_element(content[p])) {
				p++;
			}
			continue;
		}
		break;
	}
	return {SpanType::Unquoted,
	        content.substr(unquoted_argument_start, p - unquoted_argument_start)};
}

Command parse_command_invocation() {
	spans.push_back(parse_identifier());
	const size_t identifier = spans.size() - 1;
	skip_ws();
	parse_lparen();
	skip_ws();
	std::vector<size_t> arguments;
	while (true) {
		if (content[p] == ')') {
			parse_rparen();
			break;
		} else if (content[p] == '"') {
			spans.push_back(parse_quoted_argument());
			arguments.push_back(spans.size() - 1);
		} else {
			spans.push_back(parse_unquoted_argument());
			arguments.push_back(spans.size() - 1);
		}
		skip_ws();
	}

	return {identifier, arguments};
}

std::string lowerstring(const std::string &val) {
	std::string newval = val;
	std::transform(newval.begin(), newval.end(), newval.begin(),
	               [](char c) { return std::tolower(c); });
	return newval;
}

std::string repeat_string(const std::string &val, size_t n) {
	std::string newval;
	for (size_t i = 0; i < n; i++) {
		newval += val;
	}
	return newval;
}

using FormatterFunction = std::function<void(const std::vector<Command> &, std::vector<Span> &)>;

class Formatter {
  public:
	virtual ~Formatter() = default;
	virtual std::vector<std::pair<std::string, std::string>> describeCommandLine() = 0;
	virtual bool handleCommandLine(const std::string &arg,
	                               std::vector<FormatterFunction> &formatter_functions)
	    = 0;
};

class FormatterIndent : public Formatter {
	std::vector<std::pair<std::string, std::string>> describeCommandLine() override {
		return {{"-indent=STRING", "Use STRING for indentation."}};
	}

	bool handleCommandLine(const std::string &arg,
	                       std::vector<FormatterFunction> &formatter_functions) override {
		if (arg.find("-indent=") != 0) {
			return false;
		}

		std::string indent_string = arg.substr(std::string{"-indent="}.size());
		replace_all_in_string(indent_string, "\\t", "\t");
		formatter_functions.emplace_back(std::bind(run, _1, _2, indent_string));

		return true;
	};

	static void run(const std::vector<Command> &commands, std::vector<Span> &spans,
	                const std::string &indent_string) {
		int global_indentation_level = 0;
		for (auto c : commands) {
			const std::string ident = lowerstring(spans[c.identifier].data);

			if (ident == "endif" || ident == "endforeach" || ident == "endwhile"
			    || ident == "endmacro" || ident == "endfunction") {
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
						spans[next_token + 1].data
						    = (repeat_string(indent_string, indentation_level)
						       + spans[next_token + 1].data.substr(old_indentation.size()));
					}
					next_token++;
				} else if (spans[next_token].type == SpanType::Lparen) {
					break;
				} else {
					next_token++;
				}
			}

			// Walk backwards to fix comments at the same prior indentation
			// level.
			size_t last_token_on_previous_line = c.identifier - 3;
			while (true) {
				if (spans[last_token_on_previous_line].type == SpanType::Comment
				    && spans[last_token_on_previous_line - 1].type == SpanType::Space
				    && spans[last_token_on_previous_line - 2].type == SpanType::Newline) {
					spans[last_token_on_previous_line - 1].data
					    = repeat_string(indent_string, indentation_level);
					last_token_on_previous_line -= 3;
				} else if (spans[last_token_on_previous_line].type == SpanType::Space
				           && spans[last_token_on_previous_line - 1].type == SpanType::Newline) {
					last_token_on_previous_line -= 2;
				} else if (spans[last_token_on_previous_line].type == SpanType::Newline) {
					last_token_on_previous_line -= 1;
				} else {
					break;
				}
			}

			if (ident == "if" || ident == "foreach" || ident == "while" || ident == "macro"
			    || ident == "function") {
				global_indentation_level++;
			}
		}
	}
};

class FormatterIndentArgument : public Formatter {
	std::vector<std::pair<std::string, std::string>> describeCommandLine() override {
		return {
		    {"-indent-arguments=align-paren",
		     "Align arguments on continuing lines with the command's left parenthesis."},
		    {"-indent-arguments=STRING", "Use STRING for indenting arguments on continuing lines."},
		};
	}

	bool handleCommandLine(const std::string &arg,
	                       std::vector<FormatterFunction> &formatter_functions) override {

		if (arg == "-indent-arguments=align-paren") {
			formatter_functions.emplace_back(std::bind(run, _1, _2, true, ""));
			return true;
		}
		if (arg.find("-indent-arguments=") == 0) {
			std::string argument_indent_string
			    = arg.substr(std::string{"-indent-arguments="}.size());
			replace_all_in_string(argument_indent_string, "\\t", "\t");
			formatter_functions.emplace_back(std::bind(run, _1, _2, false, argument_indent_string));
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

class FormatterIndentRparen : public Formatter {
	std::vector<std::pair<std::string, std::string>> describeCommandLine() override {
		return {{"-indent-rparen=STRING", "Use STRING for indenting hanging right-parens."}};
	}

	bool handleCommandLine(const std::string &arg,
	                       std::vector<FormatterFunction> &formatter_functions) override {
		if (arg.find("-indent-rparen=") != 0) {
			return false;
		}

		std::string rparen_indent_string = arg.substr(std::string{"-indent-rparen="}.size());
		replace_all_in_string(rparen_indent_string, "\\t", "\t");

		formatter_functions.emplace_back(std::bind(run, _1, _2, rparen_indent_string));

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

class FormatterLowercaseCommands : public Formatter {
	std::vector<std::pair<std::string, std::string>> describeCommandLine() override {
		return {{"-lowercase-commands", "Lowercase command names in command invocations."}};
	}

	bool handleCommandLine(const std::string &arg,
	                       std::vector<FormatterFunction> &formatting_functions) override {
		if (arg != "-lowercase-commands")
			return false;
		formatting_functions.emplace_back(formatLowercaseCommands);
		return true;
	}

	static void formatLowercaseCommands(const std::vector<Command> &commands,
	                                    std::vector<Span> &spans) {
		for (auto c : commands) {
			spans[c.identifier].data = lowerstring(spans[c.identifier].data);
		}
	}
};

int main(int argc, char **argv) {

	std::vector<std::unique_ptr<Formatter>> formatters;
	formatters.emplace_back(new FormatterLowercaseCommands());
	formatters.emplace_back(new FormatterIndent());
	formatters.emplace_back(new FormatterIndentArgument());
	formatters.emplace_back(new FormatterIndentRparen());

	bool format_in_place = false;
	std::vector<FormatterFunction> formatting_functions;

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
		{
			std::ifstream file{filename};
			content = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
		}

		std::vector<Command> commands;
		while (true) {
			skip_ws();
			if (p >= content.size()) {
				break;
			}
			commands.push_back(parse_command_invocation());
		}

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
