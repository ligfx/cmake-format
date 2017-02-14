/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <string>
#include <vector>

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

struct Command {
	Command(size_t identifer_, std::vector<size_t> arguments_)
	    : identifier(identifer_), arguments(arguments_) {
	}
	const size_t identifier;
	const std::vector<size_t> arguments;
};

struct parseexception : public std::runtime_error {
	explicit parseexception(const std::string &message);
};

std::pair<std::vector<Span>, std::vector<Command>> parse(const std::string &content);
