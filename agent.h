#pragma once

#include "model.h"
#include "helper.h"
#include "defs.h"
#include "zobrist.h"

#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <unordered_map>

const int CHECKMATESCORE = 10000;
const int ALPHA = -10000000;
const int BETA = 10000000;

const unsigned long long QUEENSIDE_CASLTE_SQUARES = 0x707000000000707;
const unsigned long long KINGSIDE_CASTLE_SQUARES = 0xe0e000000000e0e0;
const unsigned long long MIDDLE_SQUARES = 0X1818000000;

const int fullDepthMoves = 4;
const int reductionLimit = 3;

// non-pawn material count
const int INITIAL_PIECE_MATERIAL = 2 * 3  +  2 * 3  +  2 * 5  +  1 * 9;

const std::vector<int> kingPlacementTable = {
	 2,  20,  19,  0,  0, -2,  20,  0,
	-2, -2, -2, -3, -3, -3, -2, -2,
	-4, -4, -4, -4, -4, -4, -4, -4,
	-5, -5, -5, -5, -5, -5, -5, -5,
	-5, -5, -5, -5, -5, -5, -5, -5,
	-4, -4, -4, -4, -4, -4, -4, -4,
	2, -2, -2, -3, -3, -3, -2, -2,
	 2,  20,  19,  0,  0, -2,  20,  0,
};

const std::vector<int> kingEndgamePlacementTable = {
   -2,-1, 0, 0, 0, 0,-1,-2,
   -1, 4, 4, 4, 4, 4, 4,-1,
	0, 4, 6, 6, 6, 6, 4, 0,
	0, 4, 6, 9, 9, 6, 4, 0,
	0, 4, 6, 9, 9, 6, 4, 0,
	0, 4, 6, 6, 6, 6, 4, 0,
   -1, 4, 4, 4, 4, 4, 4,-1,
   -2,-1, 0, 0, 0, 0,-1,-2,
};


const std::vector<int> knightPlacementTable = {
   -3,-1, 0, 0, 0, 0,-1,-3,
	0, 2, 2, 3, 3, 2, 2, 0,
   -1, 2, 4, 4, 4, 4, 2,-1,
	0, 2, 4, 5, 5, 4, 2, 0,
    0, 2, 4, 5, 5, 4, 2, 0,
   -1, 2, 4, 4, 4, 4, 2,-1,
	0, 2, 2, 3, 3, 2, 2, 0,
   -3,-1, 0, 0, 0, 0,-1,-3,
};

const std::vector<int> bishopPlacementTable = {
	3, 2,-1, 3, 3,-1, 2, 3,
	3, 3, 4, 2, 2, 4, 3, 3,
	1, 4, 5, 4, 4, 5, 4, 1,
	1, 3, 4, 5, 5, 4, 3, 1,
	1, 3, 4, 5, 5, 4, 3, 1,
	1, 4, 5, 4, 4, 5, 4, 1,
	3, 3, 4, 2, 2, 4, 3, 3,
	3, 2,-1, 3, 3,-1, 2, 3,
};

const std::vector<int> rookPlacementTable = {
	1, 1, 1, 6, 6, 2, 1, 1,
	1, 5, 5, 5, 5, 5, 5, 1,
	1, 3, 3, 4, 4, 3, 3, 1,
	3, 3, 3, 4, 4, 3, 3, 3,
	3, 3, 3, 4, 4, 3, 3, 3,
	1, 3, 3, 4, 4, 3, 3, 1,
	1, 5, 5, 5, 5, 5, 5, 1,
	1, 1, 1, 6, 6, 2, 1, 1,
};

const std::vector<int> pawnWhitePlacementTable = {
	 0,   0,   0,   0,   0,   0,   0,   0,
	 1,   1,   1,  -6,  -6,   1,   1,   1,
	 0,   0,   3,   3,   3,  -5,   0,   0,
	 0,   0,   3,   7,   7,  -5,  -5,   0,
	 4,   4,   4,   8,   8,   4,   4,   4,
	15,  15,  15,  15,  15,  15,  15,  15,
   100, 100, 100, 100, 100, 100, 100, 100,
	 0,   0,   0,   0,   0,   0,   0,   0,
};

const std::vector<int> pawnBlackPlacementTable = {
	0,   0,   0,   0,   0,   0,   0,   0,
  100, 100, 100, 100, 100, 100, 100, 100,
   15,  15,  15,  15,  15,  15,  15,  15,
	4,   4,   4,   8,   8,   4,   4,   4,
	0,   0,   3,   7,   7,  -5,  -5,   0,
	0,   0,   3,   3,   3,  -5,   0,   0,
	1,   1,   1,  -6,  -6,   1,   1,   1,
	0,   0,   0,   0,   0,   0,   0,   0,
};

const std::vector<int> queenPlacementTable = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 2, 2, 1, 1, 0,
	0, 2, 1, 1, 1, 2, 1, 0,
	0, 1, 1, 3, 3, 1, 1, 0,
	0, 1, 1, 3, 3, 1, 1, 0,
	0, 2, 1, 1, 1, 2, 1, 0,
	0, 1, 1, 2, 2, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};

const std::vector<int> closenessToCenterFileBonus = {
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
	1, 2, 3, 4, 4, 3, 2, 1,
};

const std::vector<int> blackPawnStormBonus = {
	0, 0, 0, 0, 0, 0, 0, 0,
	8, 8, 8, 8, 8, 8, 8, 8,
	7, 7, 7, 7, 7, 7, 7, 7,
	5, 5, 5, 5, 5, 5, 5, 5,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

const std::vector<int> whitePawnStormBonus = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	5, 5, 5, 5, 5, 5, 5, 5,
	7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8,
	0, 0, 0, 0, 0, 0, 0, 0
};

// MVV LVA [attacker][victim]
const std::vector<std::vector<int> > mvv_lva = {
	{105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600},

	{105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600}
};

const std::vector<int> material_score = {
	100,				// white pawn
	300,				// white knight
	350,				// white bishop
	500,				// white rook
	1000,				// white queen
	10000,				// white king
	-100,				// black pawn
	-300,				// black knight
	-350,				// black bishop
	-500,				// black rook
	-1000,				// black queen
	-10000				// black king
};

const std::vector<int> pawn_score = {
	90,  90,  90,  90,  90,  90,  90,  90,
	30,  30,  30,  40,  40,  30,  30,  30,
	20,  20,  20,  30,  30,  30,  20,  20,
	10,  10,  10,  20,  20,  10,  10,  10,
	 5,   5,  10,  20,  20,   5,   5,   5,
	 0,   0,   0,   5,   5,   0,   0,   0,
	 0,   0,   0, -10, -10,   0,   0,   0,
	 0,   0,   0,   0,   0,   0,   0,   0
};

const std::vector<int> knight_score = {
	-5,   0,   0,   0,   0,   0,   0,  -5,
	-5,   0,   0,  10,  10,   0,   0,  -5,
	-5,   5,  20,  20,  20,  20,   5,  -5,
	-5,  10,  20,  30,  30,  20,  10,  -5,
	-5,  10,  20,  30,  30,  20,  10,  -5,
	-5,   5,  20,  10,  10,  20,   5,  -5,
	-5,   0,   0,   0,   0,   0,   0,  -5,
	-5, -10,   0,   0,   0,   0, -10,  -5
};

const std::vector<int> bishop_score = {
	 0,   0,   0,   0,   0,   0,   0,   0,
	 0,   0,   0,   0,   0,   0,   0,   0,
	 0,   0,   0,  10,  10,   0,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,  10,   0,   0,   0,   0,  10,   0,
	 0,  30,   0,   0,   0,   0,  30,   0,
	 0,   0, -10,   0,   0, -10,   0,   0
};

const std::vector<int> rook_score = {
	50,  50,  50,  50,  50,  50,  50,  50,
	50,  50,  50,  50,  50,  50,  50,  50,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,   0,  20,  20,   0,   0,   0
};

const std::vector<int> king_score = {
	 0,   0,   0,   0,   0,   0,   0,   0,
	 0,   0,   5,   5,   5,   5,   0,   0,
	 0,   5,   5,  10,  10,   5,   5,   0,
	 0,   5,  10,  20,  20,  10,   5,   0,
	 0,   5,  10,  20,  20,  10,   5,   0,
	 0,   0,   5,  10,  10,   5,   0,   0,
	 0,   5,   5,  -5,  -5,   0,   5,   0,
	 0,   0,   5,   0, -15,   0,  10,   0
};

const std::vector<int> mirror_score = {
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1,
};

struct Node
{
	Model *currentState;
	int currentDepth;
};

struct MoveScore
{
	unsigned long long move;
	int score;
};

// no hash entry value
#define NO_HASH_ENTRY 1000000

// transposition table hash flags
#define HASH_FLAG_EXACT 0
#define HASH_FLAG_ALPHA 1
#define HASH_FLAG_BETA 2

// transposition entry
struct TT_Entry
{
	unsigned long long hash_key;
	int depth;
	int flags;
	int value;
};

class Agent
{
public:
	Agent(Zobrist *zTables);
	void UpdateFromPercepts(Percepts state);
	unsigned long long RandomAgentFunction();

	int Evaluate();
	int ForceKingToCorner(int friendlyKingSquare, int opponentKingSquare);
	int ConnectedRooksBonus(int square1, int square2);


	void PrintMoveScores(std::vector<unsigned long long> const & moves, Node* state);
	int scoreMove(int move, int depth);
	void OrderMoves(std::vector<unsigned long long>& moves, int depth);

	void ResetKilleHistoryMoves();
	void ResetPVMoves();

	void ClearTranspositionTable();
	int ProbeHash(int alpha, int beta, int depth);
	void RecordHash(int score, int depth, int hashFlag);

	void EnablePvScoring(std::vector<unsigned long long> const& moves, int ply);

	unsigned long long SearchNegamax(int depth);
	int Negamax(int alpha, int beta, int depth);
	int QuiescenceNegamax(int alpha, int beta, int depth);

	void InitializeKingBonusPassedPawnTable();
	void InitializeDistanceTables();
	
	Model mModel;

private:
	AttackTables mAttackTables;
	Zobrist *mZobristTables;
	
	int mMaxDepth;
	unsigned long long mNodes;
	unsigned long long mPrincipalVariationMove;
	unsigned long long mTTSize;
	unsigned long long mBestMoveNegamax;
	int mMaxPvLength;
	bool mFollowPvLine;
	bool mScorePV;
	int mOriginalSideToMove;
	int mPly;
	int mCurrentDepthStart;

	std::vector<std::vector<unsigned long long>> mKillerMoves;
	std::vector<std::vector<int>> mHistoryTable;

	std::unordered_map<unsigned long long, int> mMoveHashesHistory;
	std::vector<TT_Entry> mTranspositionTable;

	std::vector<unsigned long long> mPvLength;
	std::vector < std::vector<unsigned long long>> mPvTable;

	// [color][passed pawn square][king square]
	std::vector<std::vector<std::vector<int>>> mKingBonusPassedPawn;

	// king tropism distance tables
	std::vector<std::vector<int>> mDistanceTable;
	std::vector<std::vector<int>> mQueenDistanceTable;
	std::vector<std::vector<int>> mRookDistanceTable;
	std::vector<std::vector<int>> mKnightDistanceTable;

	//hash counter
	unsigned long long hashEntryCounter;


};


inline bool operator==(MoveScore a, MoveScore b)
{
	return a.score == b.score;
}

inline bool operator>=(MoveScore a, MoveScore b)
{
	return a.score >= b.score;
}

inline bool operator>(MoveScore a, MoveScore b)
{
	return a.score > b.score;
}

inline bool operator<=(MoveScore a, MoveScore b)
{
	return a.score <= b.score;
}

inline bool operator<(MoveScore a, MoveScore b)
{
	return a.score < b.score;
}