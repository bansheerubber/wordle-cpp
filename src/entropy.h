#pragma once

#include <tsl/robin_map.h>
#include <set>
#include <string>
#include <utility>

#include <iostream>

#include "key.h"

extern tsl::robin_map<std::string, double> WordFrequency;

inline float log2fast(float val) {
	union { float val; int32_t x; } u = { val };
	float log_2 = (float)(((u.x >> 23) & 255) - 128);
	u.x &= ~(255 << 23);
	u.x += 127 << 23;
	log_2 += ((-0.34484843f) * u.val + 2.02466578f) * u.val  -0.67487759f; 
	return (log_2);
} 

namespace wordle {
	inline std::pair<double, double> computeExpectedEntropy(const std::string &guess, const std::set<std::string> &words) {
		tsl::robin_map<uint16_t, double> distribution;

		double total = 0;
		for(auto &word: words) {
			double frequency = WordFrequency[word];
			distribution[computeKey(guess, word)] += frequency;
			total += frequency;
		}

		double entropy = 0;
		for(auto &[_, value]: distribution) {
			entropy -= (value / total) * log2fast(value / total);
		}

		return std::pair(entropy, words.find(guess) != words.end() ? WordFrequency[guess] / total : 0);
	}
};
