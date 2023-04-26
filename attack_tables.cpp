#include "attack_tables.h"

AttackTables::AttackTables()
{
	mBishopMasks = generateBishopAttackMaskTable();
	mRookMasks = generateRookAttackMaskTable();

	mBishopAttacks = generateBishopAttackTable();
	mRookAttacks = generateRookAttackTable();
	mPawnAttacks = generatePawnAttackTable();
	mKnightAttacks = generateKnightAttackTable();
	mKingAttacks = generateKingAttackTable();

	mPassedPawnMasks = generatePassedPawnMasks();
}

unsigned long long AttackTables::setOccupancy(int index, int bitsInMask, unsigned long long attackMask)
{
	unsigned long long occupancyMap = 0;

	// loop over the range of bits within attackMask
	for (int count = 0; count < bitsInMask; count++)
	{
		// least significant first bit index of attackMask
		int square = get_LS1B_index(attackMask);

		// clear the LS1B in the map
		clear_bit(attackMask, square);

		// make sure occupancy is on board
		if (index & (1 << count))
			// populate occupancy map
			occupancyMap |= ((unsigned long long) 1 << square);
	}

	return occupancyMap;
}


std::vector< std::vector<unsigned long long> > AttackTables::generatePawnAttackTable()
{
	std::vector <std::vector<unsigned long long > > pawnBitboards;
	pawnBitboards.resize(2);

	for (int i = 0; i < 64; i++)
	{
		unsigned long long b;
		b = calculatePawnAttackBitboard(i, white);
		pawnBitboards[white].push_back(b);

		b = calculatePawnAttackBitboard(i, black);
		pawnBitboards[black].push_back(b);

	}
	return pawnBitboards;
}

std::vector<unsigned long long> AttackTables::generateKnightAttackTable()
{
	std::vector<unsigned long long> knightAttacks;


	for (int i = 0; i < 64; i++)
	{
		unsigned long long b;
		b = calculateKnightAttackBitboard(i);
		knightAttacks.push_back(b);
	}

	return knightAttacks;
}

std::vector<unsigned long long> AttackTables::generateKingAttackTable()
{
	std::vector<unsigned long long> kingAttacks;

	for (int i = 0; i < 64; i++)
	{
		unsigned long long b;
		b = calculateKingAttackBitboard(i);
		kingAttacks.push_back(b);
	}
	return kingAttacks;
}


std::vector<unsigned long long> AttackTables::generateBishopAttackMaskTable()
{
	std::vector<unsigned long long> bishopAttacks;
	for (int i = 0; i < 64; i++)
	{
		unsigned long long b;
		b = calculateBishopAttackMask(i);
		bishopAttacks.push_back(b);
	}
	return bishopAttacks;
}

std::vector<unsigned long long> AttackTables::generateRookAttackMaskTable()
{
	std::vector<unsigned long long> rookAttacks;
	for (int i = 0; i < 64; i++)
	{
		unsigned long long b;
		b = calculateRookAttackMask(i);
		rookAttacks.push_back(b);
	}
	return rookAttacks;
}

std::vector<std::vector<unsigned long long>> AttackTables::generateBishopAttackTable()
{
	std::vector<unsigned long long> bishopMasks = generateBishopAttackMaskTable();
	
	// initialize the vector to be [64][512]
	std::vector< std::vector<unsigned long long> > bishopAttacks(64, std::vector<unsigned long long>(512, 0));


	for (int square = 0; square < 64; square++)
	{
		unsigned long long attackMask = bishopMasks[square];


		// initialize relavant occupancy bit count
		int relavantBitsCount = countBits(attackMask);

		// 2^relativeBitsCount indicies
		int occupancyIndicies = 1 << relavantBitsCount;

		// loop over occupancy indicies and populate bishopAttacks using magicIndex
		for (int index = 0; index < occupancyIndicies; index++)
		{
			// initialize current occupancy variation
			unsigned long long occupancy = setOccupancy(index, relavantBitsCount, attackMask);

			// initialize magic index
			int magicIndex = (occupancy * bishopMagicNumbers[square]) >> (64 - bishopMoveBitCount[square]);
			bishopAttacks[square][magicIndex] = calculateBishopAttackWithBlockers(square, occupancy);

		}
	}

	return bishopAttacks;
}

std::vector<std::vector<unsigned long long>> AttackTables::generateRookAttackTable()
{
	// get the rook masks
	std::vector<unsigned long long> rookMasks = generateRookAttackMaskTable();
	
	// initialize rookAttacks vector
	std::vector<std::vector<unsigned long long>> rookAttacks(64, std::vector<unsigned long long>(4096));

	for (int square = 0; square < 64; square++)
	{
		// get the mask
		unsigned long long attackMask = rookMasks[square];

		// count the bits in the mask
		int relativeBitsCount = countBits(attackMask);

		// 2^relativeBitsCount
		int occupancyIndicies = 2 << relativeBitsCount;

		// loop over occupancyIndicies and populate rookAttacks using magicIndex
		for (int index = 0; index < occupancyIndicies; index++)
		{
			// get the corresponding occupancy
			unsigned long long occupancy = setOccupancy(index, relativeBitsCount, attackMask);

			// calcuate the magic index and set the calculated attack in the table
			int magicIndex = (occupancy * rookMagicNumbers[square]) >> (64 - rookMoveBitCount[square]);
			rookAttacks[square][magicIndex] = calculateRookAttackWithBlockers(square, occupancy);

		}

	}

	return rookAttacks;
}

std::vector<std::vector<unsigned long long>> AttackTables::generatePassedPawnMasks()
{
	std::vector<std::vector<unsigned long long>> pawnMasks;
	pawnMasks.push_back({});
	pawnMasks.push_back({});

	for (int square = 0; square < 64; square++)
	{

		// white masks
		unsigned long long whiteMask = 0;
		int file = square % 8;
		int rank = square / 8;

		// the side pawns
		unsigned long long fileMasks = fileConstants[file];
		if (file != 0)
		{
			fileMasks |= fileConstants[file - 1];
		}
		if (file != 7)
		{
			fileMasks |= fileConstants[file + 1];
		}

		// set all bits > the square and are the same rank
		for (int i = square + 8; i < 64; i++)
		{
			int iSquareRank = i / 8;
			whiteMask |= rankConstants[iSquareRank];
		}
		
		whiteMask &= fileMasks;

		// black masks
		unsigned long long blackMask = 0;
		// set all bits < the square and are the same rank
		for (int i = square - 8; i >= 0; i--)
		{
			int iSquareRank = i / 8;
			blackMask |= rankConstants[iSquareRank];
		}
		blackMask &= fileMasks;

		pawnMasks[white].push_back(whiteMask);
		pawnMasks[black].push_back(blackMask);
	}

	return pawnMasks;
}



unsigned long long AttackTables::calculatePawnAttackBitboard(int square, int side)
{
	// given a square and white/black calculate pawn attack bitboard
	unsigned long long pawnAttackBitboard = 0;
	if (side == white)
	{
		set_bit(pawnAttackBitboard, ((unsigned long long)square + 7));
		set_bit(pawnAttackBitboard, ((unsigned long long)square + 9));
	}
	else
	{
		set_bit(pawnAttackBitboard, (unsigned long long)square - 7);
		set_bit(pawnAttackBitboard, (unsigned long long)square - 9);
	}

	// if square is an h pawn, we need to exclude the a-file
	if (square % 8 == 7)
	{
		pawnAttackBitboard &= ~A_FILE;
	}

	// if square is an a pawn, we need to exlude the h-file
	if (square % 8 == 0)
	{
		pawnAttackBitboard &= ~H_FILE;
	}

	return pawnAttackBitboard;
}

unsigned long long AttackTables::calculateKnightAttackBitboard(int square)
{
	// given a square  calculate knight attack bitboard
	unsigned long long knightAttackBitboard = 0;
	set_bit(knightAttackBitboard, (unsigned long long)square + 6);
	set_bit(knightAttackBitboard, (unsigned long long)square + 15);
	set_bit(knightAttackBitboard, (unsigned long long)square + 17);
	set_bit(knightAttackBitboard, (unsigned long long)square + 10);
	set_bit(knightAttackBitboard, (unsigned long long)square - 6);
	set_bit(knightAttackBitboard, (unsigned long long)square - 17);
	set_bit(knightAttackBitboard, (unsigned long long)square - 15);
	set_bit(knightAttackBitboard, (unsigned long long)square - 10);

	// if square is g-file or h-file, don't include a-file and b-file
	if (square % 8 == 6 || square % 8 == 7)
	{
		knightAttackBitboard &= ~AB_FILES;
	}

	// if square is a-file or b-file, don't include g-file and h-file
	if (square % 8 == 0 || square % 8 == 1)
	{
		knightAttackBitboard &= ~GH_FILES;
	}

	return knightAttackBitboard;
}

unsigned long long AttackTables::calculateKingAttackBitboard(int square)
{
	// given a square, calculate king attack bitboard
	unsigned long long kingAttackBitboard = 0;
	set_bit(kingAttackBitboard, (unsigned long long)square + 7);
	set_bit(kingAttackBitboard, (unsigned long long)square + 8);
	set_bit(kingAttackBitboard, (unsigned long long)square + 9);
	set_bit(kingAttackBitboard, (unsigned long long)square - 1);
	set_bit(kingAttackBitboard, (unsigned long long)square + 1);
	set_bit(kingAttackBitboard, (unsigned long long)square - 9);
	set_bit(kingAttackBitboard, (unsigned long long)square - 8);
	set_bit(kingAttackBitboard, (unsigned long long)square - 7);

	// if square is on h-file, do not include a-file
	if (square % 8 == 7)
	{
		kingAttackBitboard &= ~A_FILE;
	}

	// if square is on a-file, do not include h-file
	if (square % 8 == 0)
	{
		kingAttackBitboard &= ~H_FILE;
	}

	return kingAttackBitboard;
}

unsigned long long AttackTables::calculateBishopAttackMask(int square)
{
	unsigned long long bishopAttackBitboard = 0;
	int square_rank = square / 8;
	int square_file = square % 8;

	// up-right diagonal
	for (int rank = square_rank + 1, file = square_file + 1; rank < 7 && file < 7; rank++, file++)
	{
		set_bit(bishopAttackBitboard, rank * 8 + file);
	}

	// up-left diagonal
	for (int rank = square_rank + 1, file = square_file - 1; rank < 7 && file > 0; rank++, file--)
	{
		set_bit(bishopAttackBitboard, rank * 8 + file);
	}

	// down-right diagonal
	for (int rank = square_rank - 1, file = square_file + 1; rank > 0 && file < 7; rank--, file++)
	{
		set_bit(bishopAttackBitboard, rank * 8 + file);
	}

	// down-left diagonal
	for (int rank = square_rank - 1, file = square_file - 1; rank > 0 && file > 0; rank--, file--)
	{
		set_bit(bishopAttackBitboard, rank * 8 + file);
	}


	return bishopAttackBitboard;
}

unsigned long long AttackTables::calculateRookAttackMask(int square)
{
	unsigned long long rookAttackBitboard = 0;
	int square_rank = square / 8;
	int square_file = square % 8;

	// left-right
	for (int file = 1; file < 7; file++)
	{
		if (file == square_file)
		{
			continue;
		}

		set_bit(rookAttackBitboard, square_rank * 8 + file);
	}

	// up-down
	for (int rank = 1; rank < 7; rank++)
	{
		if (rank == square_rank)
		{
			continue;
		}

		set_bit(rookAttackBitboard, rank * 8 + square_file);
	}
	return rookAttackBitboard;
}

unsigned long long AttackTables::calculateBishopAttackWithBlockers(int square, unsigned long long blockerBitboard)
{
	unsigned long long bishopAttackBitboard = 0;
	int square_rank = square / 8;
	int square_file = square % 8;

	// up-right diagonal
	for (int rank = square_rank + 1, file = square_file + 1; rank <= 7 && file <= 7; rank++, file++)
	{
		set_bit(bishopAttackBitboard, rank * 8 + file);
		if (get_bit(blockerBitboard, rank * 8 + file))
			break;
	}

	// up-left diagonal
	for (int rank = square_rank + 1, file = square_file - 1; rank <= 7 && file >= 0; rank++, file--)
	{
		set_bit(bishopAttackBitboard, rank * 8 + file);
		if (get_bit(blockerBitboard, rank * 8 + file))
			break;
	}

	// down-right diagonal
	for (int rank = square_rank - 1, file = square_file + 1; rank >= 0 && file <= 7; rank--, file++)
	{
		set_bit(bishopAttackBitboard, rank * 8 + file);
		if (get_bit(blockerBitboard, rank * 8 + file))
			break;
	}

	// down-left diagonal
	for (int rank = square_rank - 1, file = square_file - 1; rank >= 0 && file >= 0; rank--, file--)
	{
		set_bit(bishopAttackBitboard, rank * 8 + file);
		if (get_bit(blockerBitboard, rank * 8 + file))
			break;
	}

	return bishopAttackBitboard;
}

unsigned long long AttackTables::calculateRookAttackWithBlockers(int square, unsigned long long blockerBitboard)
{
	unsigned long long rookAttackBitboard = 0;
	int square_rank = square / 8;
	int square_file = square % 8;

	// right
	for (int file = square_file + 1; file <= 7; file++)
	{
		set_bit(rookAttackBitboard, square_rank * 8 + file);
		if (get_bit(blockerBitboard, square_rank * 8 + file))
			break;
	}

	// left
	for (int file = square_file - 1; file >= 0; file--)
	{
		set_bit(rookAttackBitboard, square_rank * 8 + file);
		if (get_bit(blockerBitboard, square_rank * 8 + file))
			break;
	}

	// up
	for (int rank = square_rank + 1; rank <= 7; rank++)
	{
		set_bit(rookAttackBitboard, rank * 8 + square_file);
		if (get_bit(blockerBitboard, rank * 8 + square_file))
			break;

	}

	// down
	for (int rank = square_rank - 1; rank >= 0; rank--)
	{
		set_bit(rookAttackBitboard, rank * 8 + square_file);
		if (get_bit(blockerBitboard, rank * 8 + square_file))
			break;
	}
	return rookAttackBitboard;
}




// find appropriate magic number
unsigned long long AttackTables::findMagicNumber(int square, int relavantBits, int bishop)
{ 
	// define occupancies vector for square
	std::vector<unsigned long long> occupancies;
	occupancies.resize(4096);

	// define attack tables vector for square
	std::vector<unsigned long long> attacks;
	attacks.resize(4096);

	// define used attacks vector
	std::vector<unsigned long long> usedAttacks;
	usedAttacks.resize(4096);

	// initialize attack mask for a current piece
	unsigned long long attackMask = bishop ? calculateBishopAttackMask(square) : calculateRookAttackMask(square);

	// initialize occupancy indicies 2^relavantBits
	int occupancyIndicies = 1 << relavantBits;

	// fill the occupancies and attacks vectors
	for (int index = 0; index < occupancyIndicies; index++)
	{
		// init occupancies
		occupancies[index] = setOccupancy(index, relavantBits, attackMask);

		// initialize attacks
		if (bishop)
		{
			attacks[index] =  calculateBishopAttackWithBlockers(square, occupancies[index]);
		}
		else
		{
			attacks[index] = calculateRookAttackWithBlockers(square, occupancies[index]);
		}
	}

	// test magic numbers loop
	for (int randomCount = 0; randomCount < 10000000; randomCount++)
	{
		// generate magic number candidate
		unsigned long long magicNumber = generateMagicNumber();

		// skip bad magic numbers
		if (countBits((attackMask * magicNumber) & 0xFF00000000000000) < 6) {
			continue;
		}

		// clear the usedAttacks vector
		std::fill(usedAttacks.begin(), usedAttacks.end(), 0);

		// initialize index and fail flag
		int index, fail;

		// test magic index loop
		// try all occupancies
		for (index = 0, fail = 0; !fail && index < occupancyIndicies; index++)
		{
			// initialize magic index 
			int magicIndex = (int)((occupancies[index] * magicNumber) >> (64 - relavantBits));


			// if magic index works
			if (usedAttacks[magicIndex] == (unsigned long long)0)
			{
				// initialize used attacks
				usedAttacks[magicIndex] = attacks[index];
			}
			else if (usedAttacks[magicIndex] != attacks[index])
			{
				// magic index doesn't work
				fail = 1;
			}
		}
		// if magic number works
		if (!fail)
			return magicNumber;
	}
	// if magic number doesn't work
	std::cout << " Magic number fails!" << std::endl;
	return 0;
}

void AttackTables::initializeMagicNumbers()
{
	// loop over 64 board squares
	for (int square = 0; square < 64; square++)
	{
		// init rook magic numbers
		std::cout << "0x" << std::hex << findMagicNumber(square, rookMoveBitCount[square], rook) << std::dec << "," << std::endl;
	}

	std::cout << std::endl;
	for (int square = 0; square < 64; square++)
	{
		// init bishop magic numbers
		std::cout << "0x" << std::hex << findMagicNumber(square, bishopMoveBitCount[square], bishop) << std::dec << "," << std::endl;
	}
}