#pragma once
#include "defs.h"
#include "helper.h"

#include<vector>



// counting the number of squares a bishop can move to if no pieces block it
const std::vector <int> bishopMoveBitCount = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

// counting the number of squares a rook can move to if no pieces block it
const std::vector <int> rookMoveBitCount = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};


// rook magic numbers
const std::vector<unsigned long long> rookMagicNumbers = {
	0x8a80104000800020,
	0x140002000100040,
	0x2801880a0017001,
	0x100081001000420,
	0x200020010080420,
	0x3001c0002010008,
	0x8480008002000100,
	0x2080088004402900,
	0x800098204000,
	0x2024401000200040,
	0x100802000801000,
	0x120800800801000,
	0x208808088000400,
	0x2802200800400,
	0x2200800100020080,
	0x801000060821100,
	0x80044006422000,
	0x100808020004000,
	0x12108a0010204200,
	0x140848010000802,
	0x481828014002800,
	0x8094004002004100,
	0x4010040010010802,
	0x20008806104,
	0x100400080208000,
	0x2040002120081000,
	0x21200680100081,
	0x20100080080080,
	0x2000a00200410,
	0x20080800400,
	0x80088400100102,
	0x80004600042881,
	0x4040008040800020,
	0x440003000200801,
	0x4200011004500,
	0x188020010100100,
	0x14800401802800,
	0x2080040080800200,
	0x124080204001001,
	0x200046502000484,
	0x480400080088020,
	0x1000422010034000,
	0x30200100110040,
	0x100021010009,
	0x2002080100110004,
	0x202008004008002,
	0x20020004010100,
	0x2048440040820001,
	0x101002200408200,
	0x40802000401080,
	0x4008142004410100,
	0x2060820c0120200,
	0x1001004080100,
	0x20c020080040080,
	0x2935610830022400,
	0x44440041009200,
	0x280001040802101,
	0x2100190040002085,
	0x80c0084100102001,
	0x4024081001000421,
	0x20030a0244872,
	0x12001008414402,
	0x2006104900a0804,
	0x1004081002402
};

// bishop magic numbers
const std::vector<unsigned long long> bishopMagicNumbers = {
	0x40040844404084,
	0x2004208a004208,
	0x10190041080202,
	0x108060845042010,
	0x581104180800210,
	0x2112080446200010,
	0x1080820820060210,
	0x3c0808410220200,
	0x4050404440404,
	0x21001420088,
	0x24d0080801082102,
	0x1020a0a020400,
	0x40308200402,
	0x4011002100800,
	0x401484104104005,
	0x801010402020200,
	0x400210c3880100,
	0x404022024108200,
	0x810018200204102,
	0x4002801a02003,
	0x85040820080400,
	0x810102c808880400,
	0xe900410884800,
	0x8002020480840102,
	0x220200865090201,
	0x2010100a02021202,
	0x152048408022401,
	0x20080002081110,
	0x4001001021004000,
	0x800040400a011002,
	0xe4004081011002,
	0x1c004001012080,
	0x8004200962a00220,
	0x8422100208500202,
	0x2000402200300c08,
	0x8646020080080080,
	0x80020a0200100808,
	0x2010004880111000,
	0x623000a080011400,
	0x42008c0340209202,
	0x209188240001000,
	0x400408a884001800,
	0x110400a6080400,
	0x1840060a44020800,
	0x90080104000041,
	0x201011000808101,
	0x1a2208080504f080,
	0x8012020600211212,
	0x500861011240000,
	0x180806108200800,
	0x4000020e01040044,
	0x300000261044000a,
	0x802241102020002,
	0x20906061210001,
	0x5a84841004010310,
	0x4010801011c04,
	0xa010109502200,
	0x4a02012000,
	0x500201010098b028,
	0x8040002811040900,
	0x28000010020204,
	0x6000020202d0240,
	0x8918844842082200,
	0x4010011029020020
};


class AttackTables
{
public:
	// data members
	std::vector<unsigned long long> mBishopMasks;
	std::vector<unsigned long long> mRookMasks;

	std::vector< std::vector<unsigned long long> > mBishopAttacks;
	std::vector< std::vector<unsigned long long> > mRookAttacks;
	std::vector< std::vector<unsigned long long> > mPawnAttacks;
	std::vector<unsigned long long> mKnightAttacks;
	std::vector<unsigned long long> mKingAttacks;
	std::vector< std::vector<unsigned long long> > mPassedPawnMasks;


	AttackTables();
	// set occupancies
	unsigned long long setOccupancy(int index, int bitsInMask, unsigned long long attackMask);
	std::vector< std::vector<unsigned long long> > generatePawnAttackTable();
	std::vector<unsigned long long> generateKnightAttackTable();
	std::vector<unsigned long long> generateKingAttackTable();
	std::vector<unsigned long long> generateBishopAttackMaskTable();
	std::vector<unsigned long long> generateRookAttackMaskTable();
	std::vector< std::vector<unsigned long long> > generateBishopAttackTable();
	std::vector< std::vector<unsigned long long> > generateRookAttackTable();
	std::vector< std::vector<unsigned long long> > generatePassedPawnMasks();

	unsigned long long calculatePawnAttackBitboard(int square, int side);
	unsigned long long calculateKnightAttackBitboard(int square);
	unsigned long long calculateKingAttackBitboard(int square);
	unsigned long long calculateBishopAttackMask(int square);
	unsigned long long calculateRookAttackMask(int square);

	unsigned long long calculateBishopAttackWithBlockers(int square, unsigned long long blockerBitboard);
	unsigned long long calculateRookAttackWithBlockers(int square, unsigned long long blockerBitboard);

	inline unsigned long long getBishopAttacks(int square, unsigned long long occupancy)
	{
		// get bishop attacks assuming current board occupancy
		occupancy &= mBishopMasks[square];
		occupancy *= bishopMagicNumbers[square];
		occupancy >>= ((unsigned long long)64 - bishopMoveBitCount[square]);

		// return bishop attacks
		return mBishopAttacks[square][occupancy];
	}

	inline unsigned long long getRookAttacks(int square, unsigned long long occupancy)
	{
		// get rook attacks assuming current board occupancy
		occupancy &= mRookMasks[square];
		occupancy *= rookMagicNumbers[square];
		occupancy >>= ((unsigned long long)64 - rookMoveBitCount[square]);

		// return rook attacks
		return mRookAttacks[square][occupancy];
	}

	inline unsigned long long getQueenAttacks(int square, unsigned long long occupancy)
	{
		// resulting attack bitboard
		return getBishopAttacks(square, occupancy) | getRookAttacks(square, occupancy);

	}

	bool inline SquareAttacked(int square, int side, std::vector<unsigned long long> const& bitboards, std::vector<unsigned long long> const& occupancies)
	{
		// attacked by white pawn
		if ((side == white) && (mPawnAttacks[black][square] & bitboards[P]))
		{
			return true;
		}
		// attacked by black pawn
		if ((side == black) && (mPawnAttacks[white][square] & bitboards[p]))
		{
			return true;
		}

		// attacked by white knight
		if ((side == white) && mKnightAttacks[square] & bitboards[N])
		{
			return true;
		}

		// attacked by black knight
		if ((side == black) && mKnightAttacks[square] & bitboards[n])
		{
			return true;
		}

		// attacked by white king
		if ((side == white) && mKingAttacks[square] & bitboards[K])
		{
			return true;
		}

		// attacked by black king
		if ((side == black) && mKingAttacks[square] & bitboards[k])
		{
		return true;
		}

		// attacked by white bishops
		if ((side == white) && getBishopAttacks(square, occupancies[both]) & bitboards[B])
		{
			return true;
		}

		// attacked by black bishops
		if ((side == black) && getBishopAttacks(square, occupancies[both]) & bitboards[b])
		{
			return true;
		}

		// attacked by white rooks
		if ((side == white) && getRookAttacks(square, occupancies[both]) & bitboards[R])
		{
			return true;
		}

		// attacked by black rooks
		if ((side == black) && getRookAttacks(square, occupancies[both]) & bitboards[r])
		{
			return true;
		}

		//attacked by white queens
		if ((side == white) && getQueenAttacks(square, occupancies[both]) & bitboards[Q])
		{
			return true;
		}

		//attacked by black queens
		if ((side == black) && getQueenAttacks(square, occupancies[both]) & bitboards[q])
		{
			return true;
		}

		return false;
	}

	unsigned long long findMagicNumber(int square, int relavantBits, int bishop);
	void initializeMagicNumbers();

};

