#pragma once

#include <tsl/robin_map.h>
#include <set>
#include <string>
#include <utility>

#include <iostream>

#include "key.h"

extern tsl::robin_map<std::string, double> WordFrequency;

namespace wordle {
	inline std::pair<double, double> computeExpectedEntropy(const std::string &guess, const std::set<std::string> &words) {
		double distribution[1024];
		for(uint16_t i = 0; i < 1024; i++) {
			distribution[i] = 0.0;
		}

		double total = 0;
		for(auto &word: words) {
			distribution[computeKey(guess, word)] += WordFrequency[word];
			total += WordFrequency[word];
		}

		double entropy = 0;
		for(uint16_t i = 0; i < 1024; i++) {
			if(distribution[i] > 0) {
				entropy -= (distribution[i] / total) * log2(distribution[i] / total);
			}
		}

		return std::pair(entropy, words.find(guess) != words.end() ? WordFrequency[guess] / total : 0);
	}
};
