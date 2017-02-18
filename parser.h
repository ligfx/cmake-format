/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <stdexcept>
#include <string>
#include <vector>

enum class SpanType {
    CommandIdentifier,
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

struct parseexception : public std::runtime_error {
    explicit parseexception(const std::string &message);
};

std::vector<Span> parse(const std::string &content);
