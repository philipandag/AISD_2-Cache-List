#define _CRT_SECURE_NO_WARNINGS
#include "Input.h"

namespace Commands {
	const char* NAMES[COMMANDS_AMOUNT] = {
		"I",
		"i",
		"+",
		"-",
		".A",
		"A.",
		"R",
		"P",
	};
	extern const char* ARGS[ARGS_AMOUNT] = {
		"BEG",
		"END",
		"ALL",
	};
}

Commands::Enum getCommand(char* const buffer)
{
	char firstWord[INPUT_BUFFER_SIZE];
	for (int i = 0; i < Commands::COMMANDS_AMOUNT; i++)
	{
		if (strcmp(Commands::NAMES[i], getOneWord(buffer, firstWord)) == 0)
			return (Commands::Enum)i;
	}
	return Commands::WRONG_COMMAND;
}


bool isANumber(const char* text)
{
	while (*text != '\0')
	{
		if (*text < '0' || *text > '9')
			return false;
		text++;
	}
	return true;
}

char* getOneWord(char* const from, char* to)
{
	strcpy_s(to, INPUT_BUFFER_SIZE, from);
	int i = 0;
	while (to[i] != ' ' && to[i] != '\n' && to[i] != '\0' && to[i] != '\t')
		i++;
	to[i] = '\0';
	return to;
}

void listInit(List** list)
{
	char args[INPUT_BUFFER_SIZE];
	scanf_s("%s", args, INPUT_BUFFER_SIZE);
	if (isANumber(args))
	{
		if (*list != nullptr)
			delete (*list);
		*list = new List(atoi(args));
		List l = **list;
	}
}

void iteratorInit(List** list)
{
	char arg2[INPUT_BUFFER_SIZE];
	int iterator;
	scanf_s("%d %s", &iterator, arg2, INPUT_BUFFER_SIZE);

	if (strcmp(arg2, Commands::ARGS[Commands::Args::BEG]) == 0)
		(*list)->copyIterator(iterator, (*list)->BEG);
	else if (strcmp(arg2, Commands::ARGS[Commands::Args::END]) == 0)
		(*list)->copyIterator(iterator, (*list)->END);
	else
	{
		int pos = atoi(arg2);
		if (pos >= 0 && pos < ITERATORS)
			(*list)->copyIterator(iterator, pos);
	}
}

void iteratorForward(List** list)
{
	int iterator;
	scanf_s("%d", &iterator);
	(*list)->iteratorForward(iterator);
}

void iteratorBackward(List** list)
{
	int iterator;
	scanf_s("%d", &iterator);
	(*list)->iteratorBackward(iterator);
}

void addBefore(List** list)
{
	char iterator[INPUT_BUFFER_SIZE];
	DATA data;
	scanf_s("%s %llu", iterator, INPUT_BUFFER_SIZE, &data);
	if (strcmp(iterator, Commands::ARGS[Commands::Args::BEG]) == 0)
		(*list)->addBefore((*list)->BEG, data);
	else if (strcmp(iterator, Commands::ARGS[Commands::Args::END]) == 0)
		(*list)->addBefore((*list)->END, data);
	else if (isANumber(iterator))
	{
		int nIterator = atoi(iterator);
		if(nIterator >= 0 && nIterator < 10)
			(*list)->addBefore(nIterator, data);
	}
}

void addAfter(List** list)
{
	char iterator[INPUT_BUFFER_SIZE];
	DATA data;
	scanf_s("%s %llu", iterator, INPUT_BUFFER_SIZE, &data);
	if (strcmp(iterator, Commands::ARGS[Commands::Args::BEG]) == 0)
		(*list)->addAfter((*list)->BEG, data);
	else if (strcmp(iterator, Commands::ARGS[Commands::Args::END]) == 0)
		(*list)->addAfter((*list)->END, data);
	else if (isANumber(iterator))
	{
		int nIterator = atoi(iterator);
		if (nIterator >= 0 && nIterator < 10)
			(*list)->addAfter(nIterator, data);
	}
}

void removeAt(List** list)
{
	char arg[INPUT_BUFFER_SIZE];
	scanf_s("%s", arg, INPUT_BUFFER_SIZE);
	if (strcmp(arg, Commands::ARGS[Commands::Args::BEG]) == 0)
		(*list)->removeAtIterator((*list)->BEG);
	else if (strcmp(arg, Commands::ARGS[Commands::Args::END]) == 0)
		(*list)->removeAtIterator((*list)->END);
	else if (isANumber(arg))
	{
		int iterator = atoi(arg);
		if(iterator >= 0 && iterator < 10)
			(*list)->removeAtIterator(iterator);
	}
		
}

void printAt(List** list)
{
	char arg[INPUT_BUFFER_SIZE];
	scanf_s("%s", arg, INPUT_BUFFER_SIZE);

	if (strcmp(arg, Commands::ARGS[Commands::Args::ALL]) == 0)
		(*list)->print();
	else if (isANumber(arg))
	{
		int iterator = atoi(arg);
		if (iterator >= 0 && iterator < ITERATORS)
			(*list)->printAtIterator(iterator);
	}
}

bool performCommand(List** list)
{
	char buffer[INPUT_BUFFER_SIZE];
	scanf_s("%s", buffer, INPUT_BUFFER_SIZE);

	if (feof(stdin))
		return false;

	Commands::Enum command = getCommand(buffer);

	if (command == Commands::LIST_INIT)
		listInit(list);
	else if (*list != nullptr)
	{
		switch (command)
		{
		case Commands::LIST_INIT:
			listInit(list);
			break;
		case Commands::ITERATOR_INIT:
			iteratorInit(list);
			break;
		case Commands::ITERATOR_FORWARD:
			iteratorForward(list);
			break;
		case Commands::ITERATOR_BACKWARD:
			iteratorBackward(list);
			break;
		case Commands::ADD_BEFORE:
			addBefore(list);
			break;
		case Commands::ADD_AFTER:
			addAfter(list);
			break;
		case Commands::REMOVE_AT:
			removeAt(list);
			break;
		case Commands::PRINT_AT:
			printAt(list);
			break;
		default:
			break;
		}
	}

	return true;
}

void inputLoop(List** list)
{
	while (performCommand(list));
}