#pragma once

#include <cctype>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "command.h"

namespace wordle {
	inline std::vector<std::pair<char, CommandType>> decodeInput(std::string input) {
		std::vector<std::pair<char, CommandType>> result;
		char lastLetter = '\0';
		for(uint8_t i = 0; i < input.size(); i++) {
			char letter = input[i];
			if(letter != '-' && lastLetter != '-') {
				if(letter < 'a') { // uppercase
					result.push_back(std::pair(tolower(letter), LETTER_AT_POSITION));
				}
				else { // lowercase
					result.push_back(std::pair(tolower(letter), LETTER_IN_WORD));
				}
			}
			else if(lastLetter == '-') {
				result.push_back(std::pair(tolower(letter), EXCLUDE_LETTER));
			}
			lastLetter = letter;
		}
		return result;
	}
	
	class Knowledge {
		public:
			Knowledge();

			std::vector<double> uncertainties;
			
			void addKnowledge(std::vector<std::pair<char, CommandType>> letterList);
			std::string guess();
		
		private:
			std::set<char> exclude;
			std::set<std::pair<char, uint8_t>> atPosition;
			std::set<uint8_t> inWord[26];

			std::vector<Command> commands;

			uint8_t upperLimit[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
			uint8_t lowerLimit[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

			uint8_t guessCount = 1;

			double lastEntropy = 0;

			static double StartEntropy;

			void generateCommandList();
			void calculateLastEntropy(std::set<std::string> &words);
	};
};
