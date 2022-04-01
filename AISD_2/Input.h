#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "List.h"

#define TYPE unsigned long long
#define ITERATORS 10;

namespace Commands {
	enum Enum {
		LIST_INIT = 0,
		ITERATOR_INIT = 1,
		ITERATOR_FORWARD,
		ITERATOR_BACKWARD,
		ADD_BEFORE,
		ADD_AFTER,
		REMOVE_AT,
		PRINT_AT,
		WRONG_COMMAND,
	};
	const int AMOUNT = 8;
	extern const char* NAMES[AMOUNT];
	enum Args {
		BEG = 0,
		END,
		ALL,
	};
	extern const char* ARGS[AMOUNT];
}

const int INPUT_BUFFER_SIZE = 64;




Commands::Enum getCommand(char* buffer);

char* skipOneWord(char* buffer);

// Remember to free the returned string
char* getOneWord(char* buffer, char* oneWord);

void listInit(char* buffer, List** list);

bool performCommand(List** list);

void inputLoop(List** list);
