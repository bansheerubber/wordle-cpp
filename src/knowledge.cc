#include <algorithm>
#include <iterator>
#include <tsl/robin_map.h>

#include <iostream>

#include "entropy.h"
#include "lists.h"
#include "knowledge.h"

double wordle::Knowledge::StartEntropy = 0.0;

wordle::Knowledge::Knowledge() {
	if(Knowledge::StartEntropy == 0.0) {
		this->calculateLastEntropy(WordList);
		Knowledge::StartEntropy = this->lastEntropy;
		this->uncertainties.push_back(this->lastEntropy);
	}
	else {
		this->lastEntropy = Knowledge::StartEntropy;
		this->uncertainties.push_back(this->lastEntropy);
	}
}

void wordle::Knowledge::addKnowledge(std::vector<std::pair<char, CommandType>> letterList) {
	uint8_t letterCounts[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
	uint8_t position = 0;
	for(auto &[character, command]: letterList) {
		if(command == LETTER_AT_POSITION) {
			this->atPosition.insert(std::pair(character, position));
			letterCounts[character - 'a']++;
		}
		else if(command == LETTER_IN_WORD) {
			this->inWord[character - 'a'].insert(position);
			letterCounts[character - 'a']++;
		}
		else if(command == EXCLUDE_LETTER) {
			this->exclude.insert(character);
		}
		position++;

		if(this->upperLimit[character - 'a'] < letterCounts[character - 'a']) {
			this->upperLimit[character - 'a'] = letterCounts[character - 'a'];
		}
	}

	for(uint8_t i = 0; i < 26; i++) {
		if(letterCounts[i] > 0 && letterCounts[i] > this->lowerLimit[i]) {
			this->lowerLimit[i] = letterCounts[i];
		}
	}
}

std::string wordle::Knowledge::guess() {
	uint8_t letterCounts[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
	
	this->generateCommandList();

	std::set<std::string> results;

	Command &command = this->commands[0];
	if(command.type == LETTER_AT_POSITION) {
		results = WordsByLetterAndPosition[std::pair(command.letter, command.positions[0])];
		letterCounts[command.letter - 'a']++;
	}
	else if(command.type == LETTER_IN_WORD) {
		for(uint8_t i = 0; i < 5; i++) {
			bool quit = false;
			for(uint8_t j = 0; j < 5 && command.positions[j] != 255; j++) {
				if(i == command.positions[j]) {
					quit = true;
					break;
				}
			}

			if(quit) {
				continue;
			}
			
			results.insert(
				WordsByLetterAndPosition[std::pair(command.letter, i)].begin(),
				WordsByLetterAndPosition[std::pair(command.letter, i)].end()
			);
		}

		// above set is overly inclusive, reduce it down a little
		for(uint8_t i = 0; i < 5 && command.positions[i] != 255; i++) {
			std::set<std::string> temp;
			std::set_difference(
				results.begin(), results.end(),
				WordsByLetterAndPosition[std::pair(command.letter, command.positions[i])].begin(),
				WordsByLetterAndPosition[std::pair(command.letter, command.positions[i])].end(),
				std::inserter(temp, temp.end())
			);
			results = temp;
		}
		letterCounts[command.letter - 'a']++;
	}
	else if(command.type == EXCLUDE_LETTER) {
		for(auto &word: WordList) {
			if(word.find(command.letter) == std::string::npos) {
				results.insert(word);
			}
		}
	}

	for(uint16_t i = 1; i < this->commands.size(); i++) {
		Command &command = this->commands[i];
		if(command.type == LETTER_AT_POSITION) {
			std::set<std::string> temp;
			std::set_intersection(
				results.begin(), results.end(),
				WordsByLetterAndPosition[std::pair(command.letter, command.positions[0])].begin(),
				WordsByLetterAndPosition[std::pair(command.letter, command.positions[0])].end(),
				std::inserter(temp, temp.end())
			);
			results = temp;
			
			letterCounts[command.letter - 'a']++;
		}
		else if(command.type == LETTER_IN_WORD) {
			for(uint8_t j = 0; j < 5 && command.positions[j] != 255; j++) {
				std::set<std::string> temp;
				std::set_difference(
					results.begin(), results.end(),
					WordsByLetterAndPosition[std::pair(command.letter, command.positions[j])].begin(),
					WordsByLetterAndPosition[std::pair(command.letter, command.positions[j])].end(),
					std::inserter(temp, temp.end())
				);
				results = temp;
			}

			std::set<std::string> temp = results;
			for(auto &word: temp) {
				if(word.find(command.letter) == std::string::npos) {
					results.erase(word);
				}
			}
			
			letterCounts[command.letter - 'a']++;
		}
		else if(command.type == EXCLUDE_LETTER) {
			std::set<std::string> temp = results;
			for(auto &word: temp) {
				uint8_t count = 0;
				for(uint8_t j = 0; j < 5; j++) {
					if(word[j] == command.letter) {
						count++;
					}
				}

				if(this->upperLimit[command.letter - 'a'] < count) {
					results.erase(word);
				}
			}
		}
	}

	std::set<std::string> temp = results;
	for(uint8_t i = 0; i < 26; i++) {
		if(this->lowerLimit[i] <= 1) {
			continue;
		}

		for(auto &word: temp) {
			uint8_t count = 0;
			for(uint8_t j = 0; j < 5; j++) {
				if(word[j] == (i + 'a')) {
					count++;
				}
			}

			if(count < this->lowerLimit[i]) {
				results.erase(word);
			}
		}
		temp = results;
	}

	// std::cout << results.size() << std::endl;
	// for(auto &word: results) {
	// 	std::cout << word << " ";
	// }
	// std::cout << std::endl;

	// std::cout << this->lastEntropy << std::endl;
	// std::cout << (int)this->guessCount << std::endl;

	this->calculateLastEntropy(results);
	this->uncertainties.push_back(this->lastEntropy);

	double lowestScore = 100000.0;
	std::string guess;
	for(auto &word: WordList) {
		auto [entropy, probability] = wordle::computeExpectedEntropy(word, results);
		double delta = this->lastEntropy - entropy;

		/* double score = probability * (double)this->guessCount
			+ (1.0 - probability) * ((double)this->guessCount
				+ (0.25382263 + 5.55009018e-01 * delta + -1.10044394e-01 * pow(delta, 2) + 1.29071566e-02 * pow(delta, 3) + -5.18110556e-04 * pow(delta, 4))
			); */

		double score = probability * (double)this->guessCount
			+ (1.0 - probability) * ((double)this->guessCount + (0.42336469 + 0.27196098 * delta + -0.0074404 * pow(delta, 2)));
		if(score < lowestScore) {
			lowestScore = score;
			guess = word;
		}
	}

	// std::cout << guess << ": " << wordle::computeExpectedEntropy(guess, results).first << " " << wordle::computeExpectedEntropy(guess, results).second << std::endl;

	this->guessCount++;

	return guess;
}

void wordle::Knowledge::generateCommandList() {
	this->commands.clear();

	for(auto &[character, position]: this->atPosition) {
		this->commands.push_back(Command {
			type: LETTER_AT_POSITION,
			letter: character,
			positions: { position, },
		});
	}

	for(uint8_t i = 0; i < 26; i++) {
		std::set<uint8_t> &positionSet = this->inWord[i];
		if(positionSet.size() == 0) {
			continue;
		}

		char letter = (char)i + 'a';
		this->commands.push_back(Command {
			type: LETTER_IN_WORD,
			letter: letter,
		});

		uint8_t count = 0;
		for(uint8_t position: positionSet) {
			this->commands[this->commands.size() - 1].positions[count] = position;
			count++;
		}

		for(uint8_t j = count; j < 5; j++) {
			this->commands[this->commands.size() - 1].positions[j] = 255;
		}
	}

	for(auto character: this->exclude) {
		this->commands.push_back(Command {
			type: EXCLUDE_LETTER,
			letter: character,
		});
	}

	// for(auto &command: this->commands) {
	// 	switch(command.type) {
	// 		case LETTER_AT_POSITION: {
	// 			std::cout << "LETTER_AT_POSITION: " << command.letter << ", " << (int)command.positions[0] << std::endl;
	// 			break;
	// 		}

	// 		case LETTER_IN_WORD: {
	// 			std::cout << "LETTER_IN_WORD: " << command.letter << ", ";
	// 			for(uint8_t j = 0; j < 5; j++) {
	// 				std::cout << (int)command.positions[j] << " ";
	// 			}
	// 			std::cout << std::endl;
	// 			break;
	// 		}

	// 		case EXCLUDE_LETTER: {
	// 			std::cout << "EXCLUDE_LETTER: " << command.letter << std::endl;
	// 			break;
	// 		}
	// 	}
	// }
}

void wordle::Knowledge::calculateLastEntropy(std::set<std::string> &words) {
	double total = 0;
	for(auto &word: words) {
		total += WordFrequency[word];
	}
	
	this->lastEntropy = 0;
	for(auto &word: words) {
		double probability = WordFrequency[word] / total;
		if(probability > 0) {
			this->lastEntropy -= probability * log2fast(probability);
		}
	}
}
