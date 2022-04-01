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
}

void List::blockSizeDown(byte* block)
{
	blockSetSize(block, blockGetSize(block) - 1);
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
	datas++;
	updateIterators();
}

bool List::blockFull(byte* block) const
{
	return blockGetSize(block) >= maxBlockSize;
}

void List::blockSplit(byte* block)
{
	byte* next = blockGetNext(block);
	byte* freshBlock = appendNewBlockAfter(block);
	int size = blockGetSize(block);
	for (int i = 0; i < size/2; i++)
	{
		*blockGetP(freshBlock, i) = blockGet(block, size/2 + 1 + i);
		blockSizeDown(block);
		blockSizeUp(freshBlock);
	}

	blockSetNext(freshBlock, next);
	if(next != nullptr)
		blockSetPrevious(next, freshBlock);
	blocks++;
	updateIterators();
}

void List::blockFuse(byte* left)
{
	byte* right = blockGetNext(left);
	for (int i = 0; i < blockGetSize(right); i++)
	{
		addToBlock(left, blockGet(right, i)); // copies from right to the end of left
	}
	blockDelete(right);
}

void List::updateIterators()
{
	byte* next;
	byte* previous;
	int size;
	for (int i = 0; i < ITERATOR_COUNT - 2; i++) // minus BEG and END which are at the end of the iterators arrays
	{
		byte* block = iteratorsBlock[i];
		int pos = iteratorsPos[i];
		if ((block != nullptr && pos != -1))
		{
			size = blockGetSize(block);
			previous = blockGetPrevious(block);
			next = blockGetNext(block);
			if (pos >= size)
			{
				if (next != nullptr)
				{
					iteratorsBlock[i] = next;
					iteratorsPos[i] = 0;
				}
				else if (previous != nullptr)
				{
					iteratorsBlock[i] = previous;
					iteratorsPos[i] = blockGetSize(previous) - 1;
				}
			}
		}
	}
	if (lastBlock != nullptr && firstBlock != nullptr)
	{
		iteratorsBlock[END] = lastBlock;
		iteratorsPos[END] = blockGetSize(lastBlock) - 1;
		iteratorsBlock[BEG] = firstBlock;
		iteratorsPos[BEG] = 0;
	}
	else
	{
		resetList();
	}
}

void List::appendNewBlock()
{
	byte* block = newBlock(maxBlockSizeBytes);
	blockSetNext(lastBlock, block);
	blockSetPrevious(block, lastBlock);
	lastBlock = block;
	blocks++;
}

byte* List::getBlock(int n) const
{
	byte* current = nullptr;
	if (n <= blocks)
	{
		if (n <= blocks / 2)
		{
			current = firstBlock;
			for (int i = 0; i < n; i++)
				current = blockGetNext(current);
		}
		else
		{
			current = lastBlock;
			for (int i = blocks - 1; i > n; i--)
				current = blockGetPrevious(current);
		}
	}
	return current;
}

// ################### LIST ####################

List::List(int blockSizeBytes) :
	blockNextByte(blockSizeBytes - sizeof(byte*)), // end minus its own size to point at the beginning
	blockPreviousByte(blockSizeBytes - 2 * sizeof(byte*)), // minus own size & blockNextByte size to put it in front of blockNextByte
	blockSizeByte(blockSizeBytes - 2 * sizeof(byte*) - sizeof(int)), // again minus own size minus preceeding fields sizes
	blockArrayByte(0),
	maxBlockSizeBytes(blockSizeBytes),
	maxBlockSize((blockSizeBytes - 2 * sizeof(byte*) - sizeof(int)) / sizeof(DATA)), // (total - memory taken by two pointers and int) / data = how many
	firstBlock(nullptr),
	lastBlock(nullptr),
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

void List::setIterator(int iterator, int position)
{
	int nBlock = position / maxBlockSize;
	int nPos = position - (nBlock * maxBlockSize);
	iteratorsBlock[iterator] = getBlock(nBlock);
	iteratorsPos[iterator] = nPos;
}

void List::copyIterator(int iteratorCopy, int iteratorBase)
{
	iteratorsBlock[iteratorCopy] = iteratorsBlock[iteratorBase];
	iteratorsPos[iteratorCopy] = iteratorsPos[iteratorBase];
}

void List::blockInsert(byte* block, int pos, DATA data)
{
	for (int i = blockGetSize(block) - 1; i > pos; i--) // signed/unsigned mismatch is not a problem here, it protects from overflow when size == 0, 
	{
		*((DATA*)block + i) = *((DATA*)block + i - 1);
	}
	*((DATA*)block + pos) = data;
}


void List::addAt(byte* block, int pos, DATA data)
{
	int size;
	DATA copy;
	byte* next;
	int possibleCarries = MAX_CARRIES;

	while (possibleCarries >= 0) // it would be hard to split this function because of all the diferrent  
	{							// cases which modify local variables and the necessity of having a loop to prevent stack overflowing
		
		if (block == nullptr) // no blocks so create one and add the data to it
		{
			append(data);
			return;
		}
		size = blockGetSize(block);

		if (pos == size && pos < maxBlockSize) //if on the end of a block, just add
		{
			addToBlock(block, data);
			return;
		}

		if (possibleCarries > 0 && pos >= maxBlockSize) // carry to the next block if pos exceeds this block ( may happen when adding after the last pos )
		{
			block = blockGetNext(block);
			pos -= maxBlockSize;
			possibleCarries--;
			continue;
		}

		copy = blockGetLast(block); // copies last DATA piece not to lose it while moving
		blockInsert(block, pos, data);

		if (size + 1 <= maxBlockSize) // if nothing to carry return the copied last element 
		{
			addToBlock(block, copy);
			return;
		}

		if (possibleCarries > 0) // push the copied last element forward
		{
			next = blockGetNext(block);
			if (next != nullptr) 
			{
				block = next;
				pos = 0;
				data = copy;
				possibleCarries--;
				continue;
			}
			else
			{
				append(copy);
				return;
			}
		}
		else // if out of possible carries split the block in half to make some space 
		{
			blockSplit(block);
			addToBlock(blockGetNext(block), copy);
			return;
		}
	}
}

byte* List::appendNewBlockAfter(byte* block)
{
	byte* freshBlock = newBlock(maxBlockSize);
	blockSetNext(block, freshBlock);
	blockSetNext(freshBlock, blockGetNext(block));
	blockSetPrevious(freshBlock, block);
	if (lastBlock == block)
		lastBlock = freshBlock;
	return freshBlock;
}

void List::resetList()
{
	firstBlock = nullptr;
	lastBlock = nullptr;
	iteratorsBlock[BEG] = nullptr;
	iteratorsPos[BEG] = -1;
	iteratorsBlock[END] = nullptr;
	iteratorsPos[END] = -1;
}

void List::blockDelete(byte* block)
{
	if (blockGetPrevious(block) != nullptr)
	{
		if (blockGetNext(block) != nullptr)
		{
			blockSetPrevious(blockGetNext(block), blockGetPrevious(block));
			blockSetNext(blockGetPrevious(block), blockGetNext(block));
		}
		else
		{
			lastBlock = blockGetPrevious(block);
			blockSetNext(lastBlock, nullptr);
		}
	}
	else if (blockGetNext(block) != nullptr)
	{
		firstBlock = blockGetNext(block);
		blockSetPrevious(firstBlock, nullptr);
	}
	else
	{
		resetList();
	}

	free(block);
	updateIterators();
	blocks--;
}

void List::removeAt(byte* block, int pos)
{
	int size = blockGetSize(block);
	if (size > 1)
		for (int i = pos; i < size - 1; i++) // -1 to not set the last DATA as something off the block
			*blockGetP(block, i) = blockGet(block, i + 1);

	blockSizeDown(block);
	updateIterators();
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
		if (previous != nullptr)
		{
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
	removeAt(iteratorsBlock[iterator], iteratorsPos[iterator]);
}

void List::addBefore(int iterator, DATA data)
{
	int pos = iteratorsPos[iterator];
	addAt(iteratorsBlock[iterator], pos, data);
}

void List::addAfter(int iterator, DATA data)
{
	int pos = iteratorsPos[iterator];
	addAt(iteratorsBlock[iterator], pos + 1, data);
}

void List::iteratorForward(int iterator)
{
	if (iteratorsPos[iterator] < blockGetSize(iteratorsBlock[iterator]) - 1)
		iteratorsPos[iterator]++;
	else if (blockGetNext(iteratorsBlock[iterator]) != nullptr)
	{
		iteratorsPos[iterator] = 0;
		iteratorsBlock[iterator] = blockGetNext(iteratorsBlock[iterator]);
	}
}

void List::iteratorBackward(int iterator)
{
	if (iteratorsPos[iterator] > 0)
		iteratorsPos[iterator]--;
	else if (blockGetPrevious(iteratorsBlock[iterator]) != nullptr)
	{
		iteratorsBlock[iterator] = blockGetPrevious(iteratorsBlock[iterator]);
		iteratorsPos[iterator] = blockGetSize(iteratorsBlock[iterator]) - 1;
	}
}

DATA List::getIteratorValue(int iterator) const
{
	return *((DATA*)iteratorsBlock[iterator] + iteratorsPos[iterator]);
}

void List::printAtIterator(int iterator) const
{
	printf("%llu\n", getIteratorValue(iterator));
}

void List::append(DATA data)
{
	if (firstBlock == nullptr)
	{
		firstBlock = newBlock(maxBlockSizeBytes);
		lastBlock = firstBlock;
		this->blocks++;
	}
	if (blockFull(lastBlock))
	{
		appendNewBlock();
	}
	addToBlock(lastBlock, data);
}

void List::print() const
{
	byte* block = firstBlock;
	for (int nBlock = 0; nBlock < blocks; nBlock++)
	{
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
	byte* current = firstBlock;
	byte* next;
	while (current != nullptr)
	{
		next = blockGetNext(current);
		free(current);
		current = next;
	}
}

