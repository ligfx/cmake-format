/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <cstdlib>

#include "parser.h"

struct CMakeParser {
	CMakeParser(const std::string &content, std::vector<Span> &spans);

	void skip_ws();
	Command parse_command_invocation();
	Span parse_identifier();
	void parse_lparen();
	void parse_rparen();
	void parse_dollar_sign_expression();
	Span parse_quoted_argument();
	Span parse_unquoted_argument();

	void expect(bool condition, const std::string &description);

	const std::string &content;
	size_t p{0};
	std::vector<Span> &spans;
};

static bool isIdentifierStart(char c) {
	return std::isalpha(c) || c == '_';
}

static bool isIdentifier(char c) {
	return std::isalnum(c) || c == '_';
}

void CMakeParser::expect(bool condition, const std::string &description) {
	if (!condition) {
		printf("FAILURE: Expected %s at %zu, got:\n%s\n", description.c_str(), p,
		       content.substr(p, p + 50).c_str());
		exit(1);
	}
}

CMakeParser::CMakeParser(const std::string &content_, std::vector<Span> &spans_)
    : content(content_), spans(spans_) {
}

Span CMakeParser::parse_identifier() {
	expect(isIdentifierStart(content[p]), "identifier start");
	size_t ident_start = p;
	p++;
	while (isIdentifier(content[p])) {
		p++;
	}
	return {SpanType::Identifier, content.substr(ident_start, p - ident_start)};
}

void CMakeParser::skip_ws() {
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

Command CMakeParser::parse_command_invocation() {
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

void CMakeParser::parse_lparen() {
	expect(content[p] == '(', "left paren");
	spans.emplace_back(SpanType::Lparen, content.substr(p, 1));
	p++;
}

void CMakeParser::parse_rparen() {
	expect(content[p] == ')', "right paren");
	if (spans.back().type != SpanType::Space) {
		// HACK: Make sure right-parens are always preceded by whitespace, this makes
		// transformations easier.
		spans.emplace_back(SpanType::Space, "");
	}
	spans.emplace_back(SpanType::Rparen, content.substr(p, 1));
	p++;
}

void CMakeParser::parse_dollar_sign_expression() {
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

Span CMakeParser::parse_quoted_argument() {
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

static bool is_unquoted_element(const char c) {
	// TODO: instead of std::isspace, also need to skip anything CMake thinks is
	// whitespace (like comments)
	return !(std::isspace(c) || c == '\\' || c == '$' || c == '"' || c == '(' || c == ')');
}

Span CMakeParser::parse_unquoted_argument() {
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

std::pair<std::vector<Span>, std::vector<Command>> parse(const std::string &content) {
	std::vector<Command> commands;
	std::vector<Span> spans;
	CMakeParser parser{content, spans};

	while (true) {
		parser.skip_ws();
		if (parser.p >= content.size()) {
			break;
		}
		commands.push_back(parser.parse_command_invocation());
	}

	return std::make_pair(std::move(spans), std::move(commands));
}
