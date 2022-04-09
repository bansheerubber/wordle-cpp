#pragma once

#include <cstdint>

namespace wordle {
	enum CommandType {
		LETTER_AT_POSITION = 0,
		LETTER_IN_WORD,
		EXCLUDE_LETTER,
	};

	struct Command {
		CommandType type;
		char letter;
		uint8_t positions[5];
	};
};
