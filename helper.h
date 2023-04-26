#pragma once
#include "defs.h"

#include <vector>
#include <string>
#include <iostream>



void printBitboard(unsigned long long bitboard);

inline int countBits(unsigned long long bitboard)
{
	int count = 0;

	// clear LSB and count
	while (bitboard)
	{
		count++;
		bitboard &= bitboard - 1;
	}
	return count;
}

static const int index64[64] =
{
   0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

// get least significant 1st bit index
inline int get_LS1B_index(unsigned long long bitboard)
{
	// make sure bitboard is not equal to zero
	if (bitboard)
	{
		// count trailing bits before LS1B
		const unsigned long long debruijn64 = (0x03f79d71b4cb0a89);
		bitboard |= bitboard >> 1;
		bitboard |= bitboard >> 2;
		bitboard |= bitboard >> 4;
		bitboard |= bitboard >> 8;
		bitboard |= bitboard >> 16;
		bitboard |= bitboard >> 32;
		return index64[(bitboard * debruijn64) >> 58];
		//return countBits((bitboard & ((unsigned long long)0-bitboard)) - 1);
	}
	else
	{
		// return illegal index
		return -1;
	}
}


// generate 32-bit pseudo legal numbers
unsigned int getRandom32BitNumber();

// generate 64-bit pseudo legal numbers
unsigned long long getRandom64BitNumber();

// generate magic number candidate
unsigned long long generateMagicNumber();


void printMoveInformation(unsigned long long move);

void printMove(unsigned long long move);

void printMoveList(std::vector<unsigned long long> const& moves);

