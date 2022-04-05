#include "List.h"

// ################### BLOCK OPERATIONS ####################

byte* List::newBlock(int sizeBytes)
{
	byte* block = (byte*)malloc(sizeof(byte) * sizeBytes);
	blockSetPrevious(block, nullptr);
	blockSetNext(block, nullptr);
	blockSetSize(block, 0);

	return block;
}

void List::blockSetNext(byte* block, byte* next)
{
	*(byte**)(block + blockNextByte) = next;
}

bool List::iteratorValid(int iterator) const
{
	return iteratorsBlock[iterator] != nullptr && iteratorsPos[iterator] > -1;
}

void List::blockSetPrevious(byte* block, byte* next)
{
	*(byte**)(block + blockPreviousByte) = next;
}

void List::blockSetSize(byte* block, int size)
{
	*(int*)(block + blockSizeByte) = size;
}

byte* List::blockGetNext(byte* block) const
{
	return *(byte**)(block + blockNextByte);
}

byte* List::blockGetPrevious(byte* block) const
{
	return *(byte**)(block + blockPreviousByte);
}

int List::blockGetSize(byte* block) const
{
	return *(int*)(block + blockSizeByte);
}

void List::blockSizeUp(byte* block)
{
	blockSetSize(block, blockGetSize(block) + 1);
	datas++;
}

void List::blockSizeDown(byte* block)
{
	blockSetSize(block, blockGetSize(block) - 1);
	datas--;
}

DATA List::blockGet(byte* block, int pos)
{
	return *((DATA*)block + pos);
}

DATA List::blockGetLast(byte* block) const
{
	return *((DATA*)block + blockGetSize(block) - 1);
}

DATA* List::blockGetP(byte* block, int pos)
{
	return ((DATA*)block + pos);
}

// make sure block is not full, otherwise WILL cause overflow
void List::addToBlock(byte* block, DATA data)
{
	int size = blockGetSize(block);
	*((DATA*)block + size) = data;
	blockSizeUp(block);
	if (block == iteratorsBlock[END])
		iteratorForward(END);
}

bool List::blockFull(byte* block) const
{
	return blockGetSize(block) >= maxBlockSize;
}

void List::blockSplit(byte* block, int afterSplitBlockLastPos)
{
	byte* freshBlock = appendNewBlockAfter(block);
	int size = blockGetSize(block);

	for (int i = 0; i < ITERATOR_COUNT; i++)
	{
		if (iteratorsBlock[i] == block && iteratorsPos[i] >= afterSplitBlockLastPos)
		{
			iteratorsBlock[i] = freshBlock;
			iteratorsPos[i] -= (afterSplitBlockLastPos - 1);
		}
	}

	for (int i = 0; i < size - afterSplitBlockLastPos; i++)
	{
		addToBlock(freshBlock, blockGet(block, afterSplitBlockLastPos + i));
		blockSizeDown(block);
	}
}

void List::blockFuse(byte* left)
{
	byte* right = blockGetNext(left);
	int initialLeftSize = blockGetSize(left);
	for (int i = 0; i < blockGetSize(right); i++)
	{
		addToBlock(left, blockGet(right, i)); // copies from right to the end of left
	}
	for (int i = 0; i < ITERATOR_COUNT; i++)
	{
		if (iteratorsBlock[i] == right)
		{
			iteratorsBlock[i] = left;
			iteratorsPos[i] += initialLeftSize;
		}
	}
	blockDelete(right);
}

void List::resetIterator(int iterator)
{
	iteratorsBlock[iterator] = nullptr;
	iteratorsPos[iterator] = -1;
}

void List::prepareIteratorsToDeleteBlock(byte* block)
{
	byte* next;
	byte* previous;
	for (int i = 0; i < ITERATOR_COUNT; i++)
	{
		if (iteratorsBlock[i] == block)
		{
			next = blockGetNext(block);
			previous = blockGetPrevious(block);
			if (next != nullptr)
			{
				iteratorsBlock[i] = next;
				iteratorsPos[i] = 0;
			}
			else if (previous != nullptr)
			{
				iteratorsBlock[i] = previous;
				iteratorsPos[i] = blockGetSize(blockGetPrevious(block)) - 1;
			}
			else
			{
				resetIterator(i);
			}
		}
	}
}

void List::appendNewBlock()
{
	byte* freshBlock = newBlock(maxBlockSizeBytes);
	setEND(freshBlock);
	blocks++;
}
// ################### LIST ####################

List::List(int blockSizeBytes) :
	blockNextByte(blockSizeBytes - sizeof(byte*)), // end minus its own size to point at the beginning
	blockPreviousByte(blockSizeBytes - 2 * sizeof(byte*)), // minus own size & blockNextByte size to put it in front of blockNextByte
	blockSizeByte(blockSizeBytes - 2 * sizeof(byte*) - sizeof(int)), // again minus own size minus preceeding fields sizes
	blockArrayByte(0),
	maxBlockSizeBytes(blockSizeBytes),
	maxBlockSize((blockSizeBytes - 2 * sizeof(byte*) - sizeof(int)) / sizeof(DATA)), // (total - memory taken by two pointers and int) / data = how many
	blocks(0),
	datas(0),
	iteratorsBlock((byte**)malloc(sizeof(byte*)* ITERATOR_COUNT)),                                                           // data pieces can still fit 
	iteratorsPos((int*)malloc(sizeof(int)* ITERATOR_COUNT))
{
	for (int i = 0; i < ITERATOR_COUNT; i++)
	{
		iteratorsBlock[i] = nullptr;
		iteratorsPos[i] = -1;
	}
	if (maxBlockSize < 1)
	{
		fprintf(stderr, "Error, block size too small!");
		exit(0);
	}
}

void List::copyIterator(int iteratorCopy, int iteratorBase)
{
	iteratorsBlock[iteratorCopy] = iteratorsBlock[iteratorBase];
	iteratorsPos[iteratorCopy] = iteratorsPos[iteratorBase];
}

void List::blockArrayMoveForward(byte* block, int pos)
{
	for (int i = blockGetSize(block) - 1; i > pos; i--)
	{
		*((DATA*)block + i) = *((DATA*)block + i - 1);
	}
}

void List::blockArrayMoveBackward(byte* block, int pos)
{
	for (int i = pos; i < blockGetSize(block); i++)
	{
		*((DATA*)block + i - 1) = *((DATA*)block + i);
	}
}

void List::blockInsert(byte* block, int pos, DATA data)
{
	blockArrayMoveForward(block, pos);
	*((DATA*)block + pos) = data;
}

void List::iteratorsAdditionUpdate(byte* block, int pos)
{
	for (int i = 0; i < ITERATOR_COUNT - 2; i++)
	{
		if (iteratorsBlock[i] == block && iteratorsPos[i] >= pos)
			iteratorForward(i);
	}
}

void List::addAt(byte* block, int pos, DATA data)
{
	int size;
	DATA copy;
	byte* next;

	if (block == nullptr) // no blocks so create one and add the data to it
	{
		append(data);
		return;
	}

	size = blockGetSize(block);

	if (pos == size && pos < maxBlockSize) //if on the end of a block, just add
	{
		addToBlock(block, data);
		iteratorsAdditionUpdate(block, pos);
		return;
	}

	if (pos < maxBlockSize)
	{
		copy = blockGetLast(block); // copies last DATA piece not to lose it while moving
		blockInsert(block, pos, data);
	}
	else if (pos == maxBlockSize)
		copy = data;
	else
	{
		printf("pos > maxBlockSize");
		return; // pos to add > than maxBlockSize shouldn't happen
	}


	if (size + 1 <= maxBlockSize) // if nothing to carry put the copied last element at the end
	{
		addToBlock(block, copy);
		iteratorsAdditionUpdate(block, pos);
		return;
	}

	int afterSplitBlockLastPos = size % 2 == 0 ? size / 2 : size / 2 + 1;
	blockSplit(block, afterSplitBlockLastPos);

	if (pos <= afterSplitBlockLastPos)
		iteratorsAdditionUpdate(block, pos);

	addToBlock(blockGetNext(block), copy);


	if (pos >= afterSplitBlockLastPos) // if after the spli the new DATA lands in newly created block
		for (int i = 0; i < ITERATOR_COUNT - 2; i++) // do not touch BEG and END
			if (iteratorsBlock[i] == blockGetNext(block) && iteratorsPos[i] <= pos - afterSplitBlockLastPos)
				iteratorBackward(i);
}

byte* List::appendNewBlockAfter(byte* block)
{
	byte* freshBlock = newBlock(maxBlockSizeBytes);
	blockSetNext(freshBlock, blockGetNext(block));
	if (blockGetNext(block) != nullptr)
		blockSetPrevious(blockGetNext(block), freshBlock);
	blockSetNext(block, freshBlock);
	blockSetPrevious(freshBlock, block);

	blocks++;
	return freshBlock;
}

void List::resetList()
{
	iteratorsBlock[BEG] = nullptr;
	iteratorsPos[BEG] = -1;
	iteratorsBlock[END] = nullptr;
	iteratorsPos[END] = -1;
}

void List::setEND(byte* block)
{
	if (iteratorsBlock[END] == nullptr)
	{
		iteratorsBlock[END] = block;
		iteratorsPos[END] = blockGetSize(block) - 1;
		return;
	}

	if (block != nullptr)
	{
		blockSetNext(iteratorsBlock[END], block);
		blockSetPrevious(block, iteratorsBlock[END]);
		iteratorsBlock[END] = block;
		iteratorsPos[END] = blockGetSize(block) - 1;
		blockSetNext(block, nullptr);
	}
}

void List::setBEG(byte* block)
{
	if (iteratorsBlock[BEG] == nullptr)
	{
		iteratorsBlock[BEG] = block;
		iteratorsPos[BEG] = 0;
		return;
	}
	blockSetPrevious(iteratorsBlock[BEG], block);
	iteratorsBlock[BEG] = block;
	iteratorsPos[BEG] = 0;
	blockSetPrevious(block, nullptr);
}

void List::blockDelete(byte* block)
{
	prepareIteratorsToDeleteBlock(block);
	if (blockGetPrevious(block) != nullptr)
	{
		if (blockGetNext(block) != nullptr)
		{
			blockSetPrevious(blockGetNext(block), blockGetPrevious(block));
			blockSetNext(blockGetPrevious(block), blockGetNext(block));
		}
		else
		{
			iteratorsBlock[END] = blockGetPrevious(block);
			iteratorsPos[END] = blockGetSize(iteratorsBlock[END]) - 1;
			blockSetNext(iteratorsBlock[END], nullptr);
		}
	}
	else if (blockGetNext(block) != nullptr)
	{
		iteratorsBlock[BEG] = blockGetNext(block);
		blockSetPrevious(iteratorsBlock[BEG], nullptr);
	}
	else
	{
		resetList();
	}

	free(block);
	blocks--;
}

void List::removeAt(byte* block, int pos)
{
	if (pos < blockGetSize(block) - 1)
		blockArrayMoveBackward(block, pos + 1);

	blockSizeDown(block);

	int blockSize = blockGetSize(block);

	for (int i = 0; i < ITERATOR_COUNT; i++)
		if (iteratorsBlock[i] == block)
		{
			if ((iteratorsPos[i] > pos) || (block == iteratorsBlock[END] && iteratorsPos[i] == blockSize))
			{
				if (!iteratorBackward(i)) // move backward when deleted data in middle of a block to keep the iterator at the same value or when its the last data in the list
					resetIterator(i);// if failed
			}
			else if (iteratorsBlock[i] == block && iteratorsPos[i] == blockGetSize(block))
			{
				if (!iteratorForward(i)) // if iterator pointed to the last element and it was deleted, then move to the beg of next block
					resetIterator(i); // if failed
			}
		}



	if (blockSize == 0)
		blockDelete(block);
	else
		tryToFuse(block);
}

void List::tryToFuse(byte* block)
{
	int size = blockGetSize(block);
	byte* next = blockGetNext(block);
	byte* previous = blockGetPrevious(block);
	if (blockGetSize(block) == 0)
	{
		blockDelete(block);
	}
	else
	{
		if (next != nullptr)
		{
			if (size + blockGetSize(next) <= maxBlockSize)
			{
				blockFuse(block);
				return;
			}
		}
		else if (previous != nullptr)
		{
			blockSetNext(previous, block);
			if (size + blockGetSize(previous) <= maxBlockSize)
			{
				blockFuse(previous);
				return;
			}
		}
	}
}

void List::removeAtIterator(int iterator)
{
	if (iteratorValid(iterator))
		removeAt(iteratorsBlock[iterator], iteratorsPos[iterator]);
}

void List::addBefore(int iterator, DATA data)
{
	if (iteratorValid(iterator) || iterator == BEG || iterator == END)
		addAt(iteratorsBlock[iterator], iteratorsPos[iterator], data);
}

void List::addAfter(int iterator, DATA data)
{
	if (iteratorValid(iterator) || iterator == BEG || iterator == END)
		addAt(iteratorsBlock[iterator], iteratorsPos[iterator] + 1, data);
}

bool List::iteratorForward(int iterator)
{
	if (iteratorValid(iterator))
	{
		if (iteratorsPos[iterator] < blockGetSize(iteratorsBlock[iterator]) - 1 && iteratorsPos[iterator] < maxBlockSize - 1)
			iteratorsPos[iterator]++;
		else if (blockGetNext(iteratorsBlock[iterator]) != nullptr)
		{
			iteratorsPos[iterator] = 0;
			iteratorsBlock[iterator] = blockGetNext(iteratorsBlock[iterator]);
		}
		else
			return false;
		return true;
	}
	if (iteratorsBlock[iterator] != nullptr && iteratorsPos[iterator] == -1)
		iteratorsPos[iterator] = 0;
	return false;
}

bool List::iteratorBackward(int iterator)
{
	if (iteratorValid(iterator))
	{
		if (iteratorsPos[iterator] > 0)
			iteratorsPos[iterator]--;
		else if (blockGetPrevious(iteratorsBlock[iterator]) != nullptr)
		{
			iteratorsBlock[iterator] = blockGetPrevious(iteratorsBlock[iterator]);
			iteratorsPos[iterator] = blockGetSize(iteratorsBlock[iterator]) - 1;
		}
		else
			return false;
	}
	return true;
}

DATA List::getIteratorValue(int iterator) const
{
	return *((DATA*)iteratorsBlock[iterator] + iteratorsPos[iterator]);
}

void List::printAtIterator(int iterator) const
{
	if (iteratorValid(iterator))
		printf("%llu\n", getIteratorValue(iterator));
}

void List::append(DATA data)
{
	if (iteratorsBlock[BEG] == nullptr)
	{
		setBEG(newBlock(maxBlockSizeBytes));
		setEND(iteratorsBlock[BEG]);
		this->blocks++;
	}
	if (blockFull(iteratorsBlock[END]))
	{
		appendNewBlock();
	}
	addToBlock(iteratorsBlock[END], data);
}

void List::printIterators() const
{
	for (int i = 0; i < ITERATOR_COUNT - 2; i++)
		if (iteratorValid(i))
			printf("\ti: %d adr: %d pos: %d val: %llu\n", i, iteratorsBlock[i], iteratorsPos[i], getIteratorValue(i));
}

void List::print() const
{
	//printIterators();
	byte* block = iteratorsBlock[BEG];
	//printf("List:");
	for (int nBlock = 0; nBlock < blocks; nBlock++)
	{
		//printf("\n\tBlock %d: ", nBlock);
		for (int nItem = 0; nItem < blockGetSize(block); nItem++)
		{
			printf("%llu ", blockGet(block, nItem));
		}
		block = blockGetNext(block);
	}
	printf("\n");
}

List::~List()
{
	byte* current = iteratorsBlock[BEG];
	byte* next;
	while (current != nullptr)
	{
		next = blockGetNext(current);
		free(current);
		current = next;
	}
}

