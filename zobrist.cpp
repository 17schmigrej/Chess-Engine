#include "zobrist.h"
#include "model.h"
#include "helper.h"

Zobrist::Zobrist()
{
	InitializeRandomNumbers();
}

void Zobrist::InitializeRandomNumbers()
{
	// initialize the 12x64 random number pieces table
	int numTables = 12;
	int tableSize = 64;

	for (int i = 0; i < numTables; i++)
	{
		std::vector <unsigned long long> pieceTable;
		pieceTable = {};

		// fill the table
		for (int j = 0; j < tableSize; j++)
		{
			pieceTable.push_back(getRandom64BitNumber());
		}

		// store the piece table to the overall table
		mRandomNumberPieceTable.push_back(pieceTable);
	}

	// now do the castling tables
	tableSize = 16;
	for (int i = 0; i < tableSize; i++)
	{
		mRandomNumberCastleTable.push_back(getRandom64BitNumber());
	}

	// enpassant tables
	tableSize = 64;
	for (int i = 0; i < tableSize; i++)
	{
		mRandomNumberEnpassantTable.push_back(getRandom64BitNumber());
	}

	// side to move
	mRandomNumberSide = getRandom64BitNumber();

}

unsigned long long Zobrist::HashBoard(Model* position)
{
	unsigned long long hash = 0;

	// loop through the bitboards and hash the squares that have pieces on them
	for (int piece = P; piece <= k; piece++)
	{
		unsigned long long boardCopy = position->GetBitboards()[piece];

		while (boardCopy)
		{
			int square = get_LS1B_index(boardCopy);

			hash ^= mRandomNumberPieceTable[piece][square];

			clear_bit(boardCopy, square);
		}
	}

	// hash the castling rights
	hash ^= mRandomNumberCastleTable[position->GetCastlingRights()];

	// hash the enpassant square if it exists
	int epSquare = position->GetEnpassant();
	if (epSquare != noSquare)
	{
		hash ^= mRandomNumberEnpassantTable[epSquare];
	}

	// hash the side to move if it is black's turn
	if (position->GetSideToMove() == black)
	{
		hash ^= mRandomNumberSide;
	}

	return hash;
}

