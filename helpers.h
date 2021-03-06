/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>

#include "parser.h"

#ifdef CMAKEFORMAT_BUILD_TESTS
#include <doctest/doctest.h>
#else
#define REQUIRE(x)
#define REQUIRE_THROWS(x)
#define TOKENPASTE(x, y) x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define TEST_CASE(...) template <typename T> void TOKENPASTE2(test_, __LINE__)()
#endif

using namespace std::placeholders;

enum class SpaceBeforeParens {
    Never,
    ControlStatements,
    Always,
};

enum class LetterCase {
    Lower,
    Upper,
};

static inline void replace_all_in_string(
    std::string &main_string, const std::string &from, const std::string &to) {
    size_t pos = 0;
    while ((pos = main_string.find(from, pos)) != std::string::npos) {
        main_string.replace(pos, from.size(), to);
    }
}

static inline void replace_invisibles_with_visibles(std::string &val) {
    replace_all_in_string(val, " ", "·");
    replace_all_in_string(val, "\t", "»   ");
}

static inline std::string lowerstring(const std::string &val) {
    std::string newval = val;
    std::transform(
        newval.begin(), newval.end(), newval.begin(), [](char c) { return std::tolower(c); });
    return newval;
}

static inline std::string upperstring(const std::string &val) {
    std::string newval = val;
    std::transform(
        newval.begin(), newval.end(), newval.begin(), [](char c) { return std::toupper(c); });
    return newval;
}

static inline std::string repeat_string(const std::string &val, size_t n) {
    std::string newval;
    for (size_t i = 0; i < n; i++) {
        newval += val;
    }
    return newval;
}

static inline void delete_span(std::vector<Span> &spans, size_t span_index) {
    spans.erase(spans.begin() + span_index);
}

static inline void insert_span_at(
    const size_t &span_index, std::vector<Span> &spans, const std::vector<Span> &new_spans) {
    spans.insert(spans.begin() + span_index, new_spans.begin(), new_spans.end());
}

static inline void insert_span_at(
    const size_t &span_index, std::vector<Span> &spans, const Span &new_span) {
    return insert_span_at(span_index, spans, std::vector<Span>{new_span});
}

static inline void insert_span_before(
    size_t &span_index, std::vector<Span> &spans, const std::vector<Span> &new_spans) {
    insert_span_at(span_index, spans, new_spans);
    span_index += new_spans.size();
}

static inline void insert_span_before(
    size_t &span_index, std::vector<Span> &spans, const Span &new_span) {
    return insert_span_before(span_index, spans, std::vector<Span>{new_span});
}

static inline std::string get_command_indentation(
    const size_t &identifier_span_index, std::vector<Span> &spans) {
    const std::string ident = spans[identifier_span_index].data;

    if (identifier_span_index == 0 || spans[identifier_span_index - 1].type == SpanType::Newline) {
        return "";
    } else if (spans[identifier_span_index - 1].type == SpanType::Space) {
        return spans[identifier_span_index - 1].data;
    } else {
        throw std::runtime_error("command '" + ident + "' not preceded by space or newline: '" +
                                 spans[identifier_span_index - 1].data + "'");
    }
}

static inline void REQUIRE_PARSES(std::string original) {
    std::vector<Span> spans = parse(original);

    std::string roundtripped;
    for (const auto &s : spans) {
        roundtripped += s.data;
    }

    replace_invisibles_with_visibles(original);
    replace_invisibles_with_visibles(roundtripped);

    REQUIRE(roundtripped == original);
}

template <typename F, typename... Args>
static inline void REQUIRE_TRANSFORMS_TO(
    std::string original, std::string wanted, F transform, Args... args) {
    std::vector<Span> spans = parse(original);

    transform(spans, args...);
    std::string output;
    for (auto s : spans) {
        output += s.data;
    }

    replace_invisibles_with_visibles(output);
    replace_invisibles_with_visibles(wanted);

    REQUIRE(output == wanted);
}
