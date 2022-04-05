#pragma once
#include <iostream>

#define DATA unsigned long long
#define byte char


class List
{
private:
	const int ITERATOR_COUNT = 12; // 10 + beginning + ending
	const int MAX_CARRIES = 1; // caps the amount of blocks that will be searched for a place to put carried data before trying to split the block

	const int blockNextByte;		// these 4 hold the bytes where corresponding parts of the block begin
	const int blockPreviousByte;
	const int blockSizeByte;
	const int blockArrayByte;

	const int maxBlockSizeBytes;
	const int maxBlockSize; // max array length

	int blocks; // amount of blocks at the moment
	int datas; // amount of numbers inside all the blocks at the moment


	byte* newBlock(int sizeBytes); // returns new block of given size
	bool iteratorValid(int iterator) const;
	void setEND(byte* block);
	void setBEG(byte* block);
	void printIterators() const;
	void updateConstantIterators();
	void blockSetNext(byte* block, byte* next); // sets next block pointer of a block to given value
	void blockSetPrevious(byte* block, byte* next); // sets previous block pointer of a block to given value
	void blockSetSize(byte* block, int size); // sets block size counter to given value
	byte* blockGetNext(byte* block) const; // returns next block pointer of a block
	byte* blockGetPrevious(byte* block) const; // returns previous block pointer of a block
	int blockGetSize(byte* block) const; // returns size counter value of a block
	void blockSizeUp(byte* block); // increases block size counter by 1
	void blockSizeDown(byte* block); // decreases block size counter by 1
	bool blockFull(byte* block) const; // true if block is full, false if theres some space left
	void appendNewBlock(); // adds new block on top of the last one
	void blockSplit(byte* block, int afterSplitBlockLastPos); // splits the block into two
	void tryToFuse(byte* block);
	void blockFuse(byte* left); // will copy the next block of left and delete it
	static DATA blockGet(byte* block, int pos); // returns DATA from a block, pos is between 0 and maxBlockSize-1
	DATA blockGetLast(byte* block) const;
	static DATA* blockGetP(byte* block, int pos); // just like blockGet, but returns a pointer
	void blockRemove(byte* block, int pos);
	void addToBlock(byte* block, DATA data); // tries to add DATA at the end of block
	void iteratorsAdditionUpdate(byte* block, int pos);
	void addAt(byte* block, int pos, DATA data);
	void removeAt(byte* block, int pos);
	void resetIterator(int iterator);
	void updateIterators();
	void prepareIteratorsToDeleteBlock(byte* block);
	void resetList();
	void blockDelete(byte* block);
	void blockArrayMoveForward(byte* block, int pos); // For addition
	void blockArrayMoveBackward(byte* block, int pos); // For removal
	void blockInsert(byte* block, int pos, DATA data); // moves the array aside and inserts the data in just freed space
	byte* appendNewBlockAfter(byte* block);
public:
	const int BEG = 10;
	const int END = 11;
	byte** iteratorsBlock;
	int* iteratorsPos;

	List(int blockSizeBytes);
	void copyIterator(int iteratorCopy, int iteratorBase); // copies iteratorBase to iteratorCopy
	bool iteratorForward(int iterator); // moves iterator one DATA forward
	bool iteratorBackward(int iterator); // moves iterator one DATA backward
	DATA getIteratorValue(int iterator) const;
	void addBefore(int iterator, DATA data);
	void addAfter(int iterator, DATA data);
	void removeAtIterator(int iterator);
	void printAtIterator(int iterator) const;
	void append(DATA data);

	void print() const;
	~List();
};
