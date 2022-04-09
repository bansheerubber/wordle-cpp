#pragma once

#include <cstdint>
#include <tsl/robin_map.h>
#include <set>
#include <string>

extern std::set<std::string> WordList;
extern tsl::robin_map<std::string, double> WordFrequency;

namespace std {
	template<>
	struct hash<pair<char, uint8_t>> {
		size_t operator()(pair<char, uint8_t> const &source) const noexcept {
			return (source.first << 8) + source.second;
    }
	};

	template<>
	struct equal_to<pair<char, uint8_t>> {
		bool operator()(const pair<char, uint8_t> &x, const pair<char, uint8_t> &y) const {
			return x.first == y.first && x.second == y.second;
		}
	};
};

extern tsl::robin_map<std::pair<char, uint8_t>, std::set<std::string>> WordsByLetterAndPosition;
