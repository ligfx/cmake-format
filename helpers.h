/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <algorithm>
#include <cctype>
#include <string>

#include "parser.h"
#include "transform.h"

#ifdef CMAKEFORMAT_USE_CATCH
#include <catch.hpp>
#else
#define REQUIRE(x)
#define REQUIRE_THROWS(x)
#define TOKENPASTE(x, y) x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define TEST_CASE(x, y) template <typename T> void TOKENPASTE2(test_, __LINE__)()
#endif

static inline void replace_all_in_string(std::string &main_string, const std::string &from,
                                         const std::string &to) {
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
    std::transform(newval.begin(), newval.end(), newval.begin(),
                   [](char c) { return std::tolower(c); });
    return newval;
}

static inline std::string repeat_string(const std::string &val, size_t n) {
    std::string newval;
    for (size_t i = 0; i < n; i++) {
        newval += val;
    }
    return newval;
}

static inline void REQUIRE_PARSES(std::string original) {
    std::vector<Span> spans;
    std::vector<Command> commands;
    std::tie(spans, commands) = parse(original);

    std::string roundtripped;
    for (const auto &s : spans) {
        roundtripped += s.data;
    }

    replace_invisibles_with_visibles(original);
    replace_invisibles_with_visibles(roundtripped);

    REQUIRE(roundtripped == original);
}

static inline void REQUIRE_TRANSFORMS_TO(TransformFunction transform, std::string original,
                                         std::string wanted) {
    std::vector<Span> spans;
    std::vector<Command> commands;
    std::tie(spans, commands) = parse(original);

    transform(commands, spans);
    std::string output;
    for (auto s : spans) {
        output += s.data;
    }

    replace_invisibles_with_visibles(output);
    replace_invisibles_with_visibles(wanted);

    REQUIRE(output == wanted);
}

struct on_program_load {
    on_program_load(const std::function<void(void)> &f) {
        f();
    }
};
