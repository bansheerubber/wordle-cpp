#include <cctype>

#include "player.h"

wordle::Player::Player(std::string word) {
	this->word = word;
}

std::string wordle::Player::guess(std::string guess) {
	uint8_t letterCounts[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
	for(uint8_t i = 0; i < 5; i++) {
		letterCounts[this->word[i] - 'a']++;
	}

	for(uint8_t i = 0; i < 5; i++) {
		if(this->word[i] == guess[i]) {
			letterCounts[guess[i] - 'a']--;
		}
	}

	std::string result = "";
	uint8_t correct = 0;
	for(uint8_t i = 0; i < 5; i++) {
		if(this->word[i] == guess[i]) {
			result += toupper(guess[i]);
			correct++;
		}
		else if(letterCounts[guess[i] - 'a'] > 0) {
			result += tolower(guess[i]);
			letterCounts[guess[i] - 'a']--;
		}
		else {
			result += '-';
			result += tolower(guess[i]);
		}
	}

	if(correct != 5) {
		return result;
	}
	else {
		return "correct";
	}
}

