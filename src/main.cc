#include <cstdint>
#include <fstream>
#include <iostream>
#include <math.h>
#include <mutex>
#include <tsl/robin_map.h>
#include <set>
#include <string>
#include <thread>

#include "entropy.h"
#include "lists.h"
#include "key.h"
#include "knowledge.h"
#include "player.h"

std::set<std::string> WordList;
std::set<std::string> WordleTruth;
tsl::robin_map<std::string, double> WordFrequency;
tsl::robin_map<std::pair<char, uint8_t>, std::set<std::string>> WordsByLetterAndPosition;

std::vector<std::pair<double, uint8_t>> uncertainties;

std::mutex print;
double average = 0;
double total = 0;

void runTest(uint16_t start, uint16_t end) {
	auto it = WordleTruth.begin(), itEnd = WordleTruth.begin();
	std::advance(it, start);
	std::advance(itEnd, end);
	for(; it != itEnd; ++it) {
		// std::cout << word << ":" << std::endl;
		
		wordle::Knowledge solver;
		wordle::Player player(*it);

		std::string guess = "arose";
		bool finished = false;
		for(uint16_t i = 0; i < 6; i++) {
			std::string result = player.guess(guess);
			if(result == "correct") {
				print.lock();
				average += ((double)i + 1.0);
				total += 1.0;
				finished = true;

				for(uint8_t j = 0; j < solver.uncertainties.size(); j++) {
					uncertainties.push_back(std::pair(solver.uncertainties[j], i - j));
				}

				std::cout << *it << " " << i + 1 << std::endl;
				print.unlock();
				break;
			}
			
			solver.addKnowledge(wordle::decodeInput(result));
			guess = solver.guess();

			// std::cout << "--------------------------------------------" << std::endl;
			// std::cout << result << std::endl;
			// std::cout << guess << std::endl;
		}
		
		if(!finished) {
			print.lock();
			std::cout << "dnf " << *it << std::endl;
			print.unlock();
		}
	}
}

int main(void) {
	{
		std::ifstream file("list.txt", std::ios::in);
		std::string line;
		while(std::getline(file, line)) {
			WordList.insert(line);
		}
		file.close();
	}

	{
		std::ifstream file("wordle-truth.txt", std::ios::in);
		std::string line;
		while(std::getline(file, line)) {
			WordList.insert(line);
			WordleTruth.insert(line);
		}
		file.close();
	}

	// load frequencies
	{
		const double sigmoidCenter = 4500.0, sigmoidSlope = 0.001;
		std::ifstream file("five-letter-frequency.txt", std::ios::in);
		std::string line;
		uint64_t count = 0;
		while(std::getline(file, line)) {
			WordFrequency[line.substr(0, 5)] = 1.0 / (1.0 + exp(-sigmoidSlope * (-(double)count + sigmoidCenter)));
			count++;
		}
		file.close();
	}

	// handle frequencies for words that aren't in the frequency list
	for(auto word: WordList) {
		if(WordFrequency.find(word) == WordFrequency.end()) {
			WordFrequency[word] = 0.0;
		}
	}

	// generate indexable lists
	for(auto word: WordList) {
		for(uint8_t i = 0; i < 5; i++) {
			WordsByLetterAndPosition[std::pair(word[i], i)].insert(word);
		}
	}

	wordle::Knowledge solver;
	while(true) {
		std::cout << "Guess: ";
		std::string result;
		std::cin >> result;

		solver.addKnowledge(wordle::decodeInput(result));
		std::cout << solver.guess() << std::endl;
	}

	// std::vector<std::thread> threads;
	// for(uint8_t i = 0; i < 8; i++) {
	// 	if(i != 7) {
	// 		threads.push_back(std::thread(runTest, WordleTruth.size() / 8 * i, WordleTruth.size() / 8 * (i + 1)));
	// 	}
	// 	else {
	// 		threads.push_back(std::thread(runTest, WordleTruth.size() / 8 * i, WordleTruth.size()));
	// 	}
	// }

	// for(auto &thread: threads) {
	// 	thread.join();
	// }

	// std::cout << "average score: " << average / total << std::endl;

	// std::ofstream file("uncertainties");
	// for(auto &[uncertainty, score]: uncertainties) {
	// 	file << uncertainty << ", " << (int)score << std::endl;
	// }
	// file.close();

	return 0;
}
