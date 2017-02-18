/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#include <cctype>

#include "cmListFileLexer.h"
#include "helpers.h"
#include "parser.h"

parseexception::parseexception(const std::string &message) : std::runtime_error{message} {
}

struct Lexer {

    Lexer(const std::string &data) {
        lexer = cmListFileLexer_New();
        if (!lexer) {
            throw std::runtime_error("couldn't allocate cmListFileLexer");
        }

        (void)cmListFileLexer_SetString(lexer, data.c_str());
        // TODO: handle result

        advance();
    }
    ~Lexer() {
        cmListFileLexer_Delete(lexer);
    }

    void advance() {
        token = cmListFileLexer_Scan(lexer);
    }

    cmListFileLexer_Token *token;

  private:
    cmListFileLexer *lexer;
};

void skip_whitespace(std::vector<Span> &spans, Lexer &lexer) {
    while (true) {
        if (!lexer.token) {
            break;
        } else if (cmListFileLexer_Token_Space == lexer.token->type) {
            spans.emplace_back(SpanType::Space, lexer.token->text);
            lexer.advance();

        } else if (cmListFileLexer_Token_Newline == lexer.token->type) {
            spans.emplace_back(SpanType::Newline, lexer.token->text);
            lexer.advance();
            // HACK: Make sure newlines are always followed by whitespace, this makes
            // transformations easier.
            if (!lexer.token || lexer.token->type != cmListFileLexer_Token_Space) {
                spans.emplace_back(SpanType::Space, "");
            }

        } else if (cmListFileLexer_Token_Comment == lexer.token->type) {
            spans.emplace_back(SpanType::Comment, lexer.token->text);
            lexer.advance();

        } else {
            break;
        }
    }
}

std::string tokentostring(const cmListFileLexer_Token *token) {
    if (!token)
        return "end-of-input";
    return cmListFileLexer_GetTypeAsString(nullptr, token->type);
}

void expecttokentype(const std::string &description, const cmListFileLexer_Token *token,
    const std::vector<cmListFileLexer_Type> &wanted_types) {
    for (auto w : wanted_types) {
        if (token && token->type == w)
            return;
    }
    throw parseexception("expected " + description + ", got " + tokentostring(token) + ": '" +
                         (token ? std::string{token->text} : "") + "'");
}

void parse_argument(std::vector<Span> &spans, Lexer &lexer) {
    if (lexer.token && lexer.token->type == cmListFileLexer_Token_ParenLeft) {
        spans.emplace_back(SpanType::Lparen, lexer.token->text);
        lexer.advance();

        while (true) {
            skip_whitespace(spans, lexer);
            if (lexer.token && lexer.token->type == cmListFileLexer_Token_ParenRight) {
                if (spans.back().type != SpanType::Space) {
                    // HACK: Make sure right-parens are always preceded by whitespace, this makes
                    // transformations easier.
                    spans.emplace_back(SpanType::Space, "");
                }
                spans.emplace_back(SpanType::Rparen, lexer.token->text);
                lexer.advance();
                break;
            }
            parse_argument(spans, lexer);
        }
    } else if (lexer.token && lexer.token->type == cmListFileLexer_Token_Identifier) {
        spans.emplace_back(SpanType::Unquoted, lexer.token->text);
        lexer.advance();
    } else if (lexer.token && lexer.token->type == cmListFileLexer_Token_ArgumentUnquoted) {
        spans.emplace_back(SpanType::Unquoted, lexer.token->text);
        lexer.advance();
    } else if (lexer.token && lexer.token->type == cmListFileLexer_Token_ArgumentQuoted) {
        spans.emplace_back(SpanType::Quoted, "\"" + std::string{lexer.token->text} + "\"");
        lexer.advance();
    } else {
        expecttokentype("argument or rparen", lexer.token, {});
    }
}

std::pair<std::vector<Span>, std::vector<Command>> parse(const std::string &content) {
    std::vector<Command> commands;
    std::vector<Span> spans;

    Lexer lexer{content};

    while (true) {
        skip_whitespace(spans, lexer);
        if (!lexer.token) {
            break;
        }

        // TODO: investigate replacing direct use of tokens with a shim that:
        // - treats EOI as another token type
        // - doesn't use such frickin' long enum names
        expecttokentype(
            "whitespace or identifier", lexer.token, {cmListFileLexer_Token_Identifier});
        // HACK: Make sure command identifiers are always preceded by whitespace
        if (spans.size() == 0 || spans.back().type != SpanType::Space) {
            spans.emplace_back(SpanType::Space, "");
        }
        spans.emplace_back(SpanType::CommandIdentifier, lexer.token->text);
        commands.push_back({spans.size() - 1, {}});
        lexer.advance();

        skip_whitespace(spans, lexer);

        expecttokentype("whitespace or left paren", lexer.token, {cmListFileLexer_Token_ParenLeft});
        spans.emplace_back(SpanType::Lparen, lexer.token->text);
        lexer.advance();

        while (true) {
            skip_whitespace(spans, lexer);
            if (lexer.token && lexer.token->type == cmListFileLexer_Token_ParenRight) {
                spans.emplace_back(SpanType::Rparen, lexer.token->text);
                lexer.advance();
                break;
            }
            parse_argument(spans, lexer);
        }
    }

    return std::make_pair(std::move(spans), std::move(commands));
}

TEST_CASE("Parses CMake code", "[parsing]") {
    REQUIRE_PARSES(R"(
cmake_command_without_arguments()
_underscore_cmake_command()
UPPERCASE_CMAKE_COMMAND()
c3e_c5d_w2h_n5s()
simple_cmake_command(
	simple_argument
	UPPERCASE_ARGUMENT
	_argument_starting_with_underscore
	-argument-with-dashes
	a6t_w2h_n5s
	1234567890
	argument;with;semicolons
	argument/with/slashes
	argument\\with\\backslashes
	argument_with_\(parentheses\)
	"quoted argument"
	bare_argument_with_"quoted argument"
	# comment
	${bare_variable_reference}
	${make_style_base_variable_reference}
	${variable_reference_${embedded_variable_reference}}
	$<$<generator_expression>:value can be anything! ${variable_reference}>
)
)");
}

TEST_CASE("Doesn't hang on unbalanced parentheses", "[parsing]") {
    REQUIRE_THROWS(parse(R"(command()"));
}

TEST_CASE("Parses bare parentheses in arguments", "[parsing]") {
    REQUIRE_PARSES("simple_cmake_command(some (bare (parentheses)) here)");
}
