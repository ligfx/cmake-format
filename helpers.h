/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include <algorithm>
#include <cctype>
#include <string>

static inline void replace_all_in_string(std::string &main_string, const std::string &from,
                                         const std::string &to) {
	size_t pos = 0;
	while ((pos = main_string.find(from, pos)) != std::string::npos) {
		main_string.replace(pos, from.size(), to);
	}
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
