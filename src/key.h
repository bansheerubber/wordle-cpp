#pragma once

#include <cstdint>
#include <string>
#include <iostream>

inline uint16_t computeKey(const std::string &guess, const std::string &word) {
	char letterCounts[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
	for(uint8_t i = 0; i < 5; i++) {
		if(guess[i] != word[i]) {
			letterCounts[word[i] - 'a']++;
		}
	}

	uint16_t key = 0;
	for(uint8_t i = 0; i < 5; i++) {
		if(guess[i] == word[i]) {
			key = key | (3 << (i * 2));
		}
		else if(letterCounts[guess[i] - 'a'] > 0) {
			key = key | (2 << (i * 2));
			letterCounts[guess[i] - 'a']--;
		}
		else {
			key = key | (1 << (i * 2));
		}
	}

	return key;
}

inline std::string keyToString(uint16_t key) {
	std::string output = "";
	for(uint8_t i = 0; i < 5; i++) {
		uint8_t value = (key >> (i * 2)) & 0b11;
		if(value == 3) {
			output += 'G';
		}
		else if(value == 2) {
			output += 'Y';
		}
		else if(value == 1) {
			output += 'R';
		}
	}
	return output;
}
