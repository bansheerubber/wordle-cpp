#pragma once

#include <string>

namespace wordle {
	class Player {
		public:
			Player(std::string word);

			std::string guess(std::string guess);
		
		private:
			std::string word;
	};
};
