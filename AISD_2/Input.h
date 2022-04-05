#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "List.h"

#define TYPE unsigned long long
#define ITERATORS 10

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
	const int COMMANDS_AMOUNT = 8;
	const int ARGS_AMOUNT = 8;
	extern const char* NAMES[ARGS_AMOUNT];
	enum Args {
		BEG = 0,
		END,
		ALL,
	};
	extern const char* ARGS[ARGS_AMOUNT];
}

const int INPUT_BUFFER_SIZE = 64;




Commands::Enum getCommand(char* buffer);

bool isANumber(const char* text);

char* getOneWord(char* buffer, char* oneWord);

void listInit(char* buffer, List** list);
void iteratorInit(List** list);
void iteratorForward(List** list);
void iteratorBackward(List** list);
void addBefore(List** list);
void addAfter(List** list);
void removeAt(List** list);
void printAt(List** list);

bool performCommand(List** list);
void inputLoop(List** list);
