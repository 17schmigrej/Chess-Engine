#include "agent.h"

Agent::Agent(Zobrist* zTables) : mModel(&mAttackTables, zTables)
{
	mMaxDepth = 8;
	mCurrentDepthStart = 1;
	mZobristTables = zTables;
	mModel.SetHash(mZobristTables->HashBoard(&mModel));
	mNodes = 0;
	mPrincipalVariationMove = 0;
	mBestMoveNegamax = 0;
	mPly = 0;
	mOriginalSideToMove = white;

	// initialize the killer moves size
	int ply = 64;
	int maxKillerMoves = 2;
	for (int i = 0; i < maxKillerMoves; i++)
	{
		std::vector<unsigned long long> moves(ply, 0);
		mKillerMoves.push_back(moves);
	}

	// initialize the history table size
	int numPieces = 12;
	int numSquares = 64;
	for (int i = 0; i < numPieces; i++)
	{
		std::vector<int> history(numSquares, 0);
		mHistoryTable.push_back(history);
	}

	// initialize the transposition table size
	mTTSize = 0x1000;
	for (int i = 0; i < mTTSize; i++)
	{
		TT_Entry tt;
		tt.depth = 0;
		tt.flags = 0;
		tt.hash_key = 0;
		tt.value = 0;
		mTranspositionTable.push_back(tt);
	}
	//mTranspositionTable.reserve(mTTSize);

	mMaxPvLength = 64;
	for (int i = 0; i < mMaxPvLength; i++)
	{
		std::vector<unsigned long long> moves(mMaxPvLength, 0);
		mPvTable.push_back(moves);
		mPvLength.push_back(0);
	}

	mFollowPvLine = false;
	mScorePV = false;


	InitializeKingBonusPassedPawnTable();
	InitializeDistanceTables();

	hashEntryCounter = 0;
}

void Agent::UpdateFromPercepts(Percepts state)
{
	mModel.SetBitboards(state.bitboards);
	mModel.SetOccupancies(state.occupancies);
	mModel.SetSideToMove(state.sideToMove);
	mModel.SetEnpassant(state.enPassant);
	mModel.SetCastlingRights(state.castlingRights);
	mModel.SetHalfMoveClock(state.halfMoveClock);
	mModel.SetFullMoves(state.fullMoves);
	mModel.SetHash(state.hash);

	mOriginalSideToMove = mModel.GetSideToMove();

	for (const auto& i : state.movesHistory)
	{
		mMoveHashesHistory[i.first] = 1;
	}

}

unsigned long long Agent::RandomAgentFunction()
{
	// will pick a random move in the position and return it
	std::vector<unsigned long long> moves;
	mModel.generateMoves(moves);

	// get index of all legal positions
	std::vector<int> indices;
	for (int i = 0; i < moves.size(); i++)
	{
		mModel.MakeMove(moves[i], allMoves);
		if (mModel.IsIllegal())
		{
			continue;
		}
		indices.push_back(i);
	}

	int randomIndex = std::rand() % indices.size();

	return moves[indices[randomIndex]];
}



void Agent::PrintMoveScores(std::vector<unsigned long long> const& moves, Node* state)
{
	std::cout << "Printing move scores" << std::endl;
	for (int i = 0; i < moves.size(); i++)
	{
		std::cout << "Move: ";
		printMove(moves[i]);
		std::cout << " Score: " << scoreMove(moves[i], state->currentDepth) << std::endl;
	}
}


int Agent::scoreMove(int move, int depth)
{
	// score capture move

	if (mScorePV)
	{
		// make sure we have the right PV move
		if (mPvTable[0][depth] == move)
		{
			// disable the scorePV flag
			mScorePV = false;

			// return a high priority
			return 20000;
		}
	}

	if (decode_capture_flag(move))
	{
		
		// initialize target piece
		int target_piece = P;

		// we need to determine which side white/black we need to pop the bit from the bitboard
		int startPiece, endPiece;


		if (mModel.GetSideToMove() == white)
		{
			startPiece = p;
			endPiece = k;
		}
		else
		{
			startPiece = P;
			endPiece = K;
		}
		std::vector<unsigned long long> bitboards = mModel.GetBitboards();
		// now loop over the bitboards and clear the bit that had the captured piece
		for (int i = startPiece; i <= endPiece; i++)
		{

			if (get_bit(bitboards[i], decode_destination_square(move)))
			{
				target_piece = i;
				break;
			}
		}


		// score by MVV LVA lookup
		return mvv_lva[decode_piece_type(move)][target_piece] + 10000;
	}

	// score quiet move
	else
	{
		// score first killer
		if (mKillerMoves[0][depth] == move)
		{
			return 9000;
		}

		// score second killer
		else if (mKillerMoves[1][depth] == move)
		{
			return 8000;
		}
		else if (decode_castling_flag(move))
		{
			// score castling move
			return 15000;
		}
		// score history move
		else
		{
			return mHistoryTable[decode_piece_type(move)][decode_destination_square(move)];
		}
	}
	return 0;
}

void Agent::OrderMoves(std::vector<unsigned long long>& moves, int depth)
{
	// move scores
	//std::vector<int> movesScores(moves.size(), 0);

	std::vector<MoveScore> moveScores;
	// score all the moves within a move list
	// zip up
	for (int count = 0; count < moves.size(); count++)
	{
		MoveScore a;
		a.score = scoreMove(moves[count], depth);
		a.move = moves[count];
		moveScores.push_back(a);
		//movesScores[count] = scoreMove(moves[count], state);
	}

	// sort in descending order
	std::sort(moveScores.begin(), moveScores.end(), [](const MoveScore a, MoveScore b) {
		return b < a;
	});

	// unpack
	for (int i = 0; i < moves.size(); i++)
	{
		moves[i] = moveScores[i].move;
	}
}

void Agent::ResetKilleHistoryMoves()
{
	// reset the killer moves
	for (int i = 0; i < mKillerMoves.size(); i++)
	{
		for (int j = 0; j < mKillerMoves[i].size(); j++)
		{
			mKillerMoves[i][j] = 0;
		}
	}

	// reset the history table
	for (int i = 0; i < mHistoryTable.size(); i++)
	{
		for (int j = 0; j < mHistoryTable[i].size(); j++)
		{
			mHistoryTable[i][j] = 0;
		}
	}


}

void Agent::ResetPVMoves()
{

	// reset pv length
	for (int i = 0; i < mPvLength.size(); i++)
	{
		mPvLength[i] = 0;
	}

	// reset pv table
	for (int i = 0; i < mPvTable.size(); i++)
	{
		for (int j = 0; j < mPvTable[i].size(); j++)
		{
			mPvTable[i][j] = 0;
		}
	}
}

void Agent::ClearTranspositionTable()
{
	//mTranspositionTable.clear();
	//mTranspositionTable.reserve(mTTSize);
	for (int i = 0; i < mTranspositionTable.size(); i++)
	{
		mTranspositionTable[i].hash_key = 0;
		mTranspositionTable[i].depth = 0;
		mTranspositionTable[i].flags = 0;
		mTranspositionTable[i].value = 0;
	}
}

int Agent::ProbeHash(int alpha, int beta, int depth)
{
	// get the hash entry for the current position if it exists
	TT_Entry* entry = &mTranspositionTable[mModel.mHash % mTTSize];

	// verify that we have the right position
	if (entry->hash_key == mModel.mHash)
	{
		// verify the depth of the entry
		if (entry->depth >= depth)
		{
			// match what the entry has
			if (entry->flags == HASH_FLAG_EXACT)
			{
				//std::cout << "exact score: " << std::endl;
				return entry->value;
			}
			if ((entry->flags == HASH_FLAG_ALPHA) &&
				(entry->value <= alpha))
			{
				//std::cout << "alpha score: " << std::endl;
				return alpha;
			}
			if ((entry->flags == HASH_FLAG_BETA) &&
				(entry->value >= beta))
			{
				//std::cout << "beta score: " << std::endl;
				return beta;
			}
		}
	}
	// return something out of bounds
	return NO_HASH_ENTRY;
}

void Agent::RecordHash(int score, int depth, int hashFlag)
{
	// get the correct entry for the hash
	TT_Entry* entry = &mTranspositionTable[mModel.mHash % mTTSize];

	// write the entry data
	entry->hash_key = mModel.mHash;
	entry->value = score;
	entry->flags = hashFlag;
	entry->depth = depth;
}

void Agent::EnablePvScoring(std::vector<unsigned long long> const& moves, int ply)
{
	// disable following PV
	mFollowPvLine = false;

	// loop over the moves
	for (int i = 0; i < moves.size(); i++)
	{
		// make sure we hit PV move
		if (mPvTable[0][ply] == moves[i])
		{
			mScorePV = true;
			mFollowPvLine = true;
		}
	}
}

unsigned long long Agent::SearchNegamax(int depth)
{
	int score = 0;
	mNodes = 0;
	ResetKilleHistoryMoves();
	ResetPVMoves();
	mFollowPvLine = false;
	mScorePV = false;

	int goalNodesSearched = 3000000;

	// get the time in ms
	unsigned long long time_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();


	for (int i = 1; i <= depth; i++)
	{
		//hashEntryCounter = 0;
		ClearTranspositionTable();
		ResetKilleHistoryMoves();
		mNodes = 0;
		// enable follow PV flag
		mFollowPvLine = true;
		mCurrentDepthStart = i;

		score = Negamax(-50000, 50000, i);
		std::cout << "depth " << i << " score " << score << " nodes " << mNodes << " pv ";

		// loop oever PV line
		for (int count = 0; count < mPvLength[0]; count++)
		{
			printMove(mPvTable[0][count]);
			std::cout << " ";
		}
		std::cout << std::endl;

		// check the time
		unsigned long long time_end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();


		// time controls, allow more search depth if done quickly
		if ((time_end - time_start < 12000 || depth <= 14 ) && i == depth)
		{
			depth++;
		}

		if (mNodes >= goalNodesSearched)
		{
			depth--;
		}

		if (depth == 32)
		{
			depth = 0;
		}

		// if we found a checkmate score for the side to move, we just cut the search early
		if (score > 45000)
		{
			break;
		}
	}
	return mPvTable[0][0];
}


int Agent::Negamax(int alpha, int beta, int depth)
{
	// define score variable
	int score;

	// set the hash flag for the transposition table
	int hashf = HASH_FLAG_ALPHA;

	// read transposition table
	
	
	
	// transposition table lookup
	if ((score = ProbeHash(alpha, beta, depth)) != NO_HASH_ENTRY)
	{
		// return the value if the position has been searched before
		//std::cout << "here" << std::endl;

		if (score == 49000)
			return score - mPly;
		if (score == -49000)
			return score + mPly;

		return score;
	}
	

	// if this position has been repeated before, return 0 (avoids repeats if winning)
	if (mMoveHashesHistory.count(mModel.GetHash()) && mPly != 0)
	{
		return 0;
	}

	// init PV length
	mPvLength[mPly] = mPly;

	// recursion escape condition
	if (depth == 0 || mPly == 32)
	{
		//return EvalNegamax();
		return QuiescenceNegamax(alpha, beta, depth);
	}

	// increment nodes count
	mNodes++;


	// number of moves searched in a move list
	int movesSearched = 0;

	// count the legal moves
	int legalMoves = 0;

	// in-check state of the side to move
	int sideToMove = mModel.mSideToMove;
	unsigned long long kingBitboard = (sideToMove == white) ? mModel.mBitboards[K] : mModel.mBitboards[k];
	int inCheck = mAttackTables.SquareAttacked(get_LS1B_index(kingBitboard), sideToMove ^ 1,
		mModel.mBitboards, mModel.mOccupancies);

	if (inCheck)
	{
		depth++;
	}
	
	
	// Null move pruning
	if (depth >= 3 && !inCheck && mPly && !mModel.OnlyPawns())
	{
		// make a null move (switch side to move)
		// literally give an opponent a free move

		mModel.mSideToMove ^= 1;

		// keep the old enpassant square
		int oldEnpassant = mModel.mEnPassant;

		// keep the old hash
		int oldHash = mModel.mHash;

		// undo the enpassant hash
		if (mModel.mEnPassant != noSquare)
		{
			mModel.mHash ^= mZobristTables->mRandomNumberEnpassantTable[mModel.mEnPassant];
		}

		// reset the enpassant square
		mModel.mEnPassant = noSquare;

		// re-hash the side change
		mModel.mHash ^= mZobristTables->mRandomNumberSide;

		// do the search with reduced depth to find beta cutoff
		// (depth - 1 - R, where R is a reduction limit)
		score = -Negamax(-beta, -beta + 1, depth - 1 - 2);

		// restore board state
		mModel.mEnPassant = oldEnpassant;
		mModel.mHash = oldHash;
		mModel.mSideToMove ^= 1;

		// fail hard beta cutoff
		if (score >= beta)
		{
			return beta;
		}
	}
	
	

	// create move list instance
	std::vector<unsigned long long> moves;
	mModel.generateMoves(moves);

	// If we are on a PV line
	if (mFollowPvLine)
	{
		EnablePvScoring(moves, mPly);
	}

	// sort the moves
	OrderMoves(moves, mPly);

	for (int count = 0; count < moves.size(); count++)
	{
		// increment ply
		mPly++;

		// make the move
		mModel.MakeMove(moves[count], allMoves);

		// make sure to make only legal moves
		if (mModel.IsIllegal())
		{
			// take back the move
			mModel.UnmakeMove(moves[count]);
			mPly--;

			continue;
		}

		// increment the number of legal moves counted
		legalMoves++;

		if (mCurrentDepthStart > 6)
		{
			// full depth search
			if (movesSearched == 0)
			{
				// score current move
				score = -Negamax(-beta, -alpha, depth - 1);
			}
			// Late Move Reduction (LMR) assume that the first handfull of moves are the best, search the rest with reduced depth
			else
			{

				// condition to consider LMR
				if (movesSearched >= fullDepthMoves &&
					depth >= reductionLimit &&
					inCheck == false && // neither king was put into check
					mCurrentDepthStart != depth && // root node
					decode_capture_flag(moves[count]) == 0 && // no captures
					decode_promoted_piece_type(moves[count]) == 0 && // no promotions
					mKillerMoves[0][mPly] != moves[count] && // not a killer move
					mKillerMoves[1][mPly] != moves[count])
				{
					// search move with a reduced depth
					score = -Negamax(-alpha - 1, -alpha, depth - 2);
				}
				else
				{
					// hack to ensure a full depth search is done
					score = alpha + 1;
				}

				// PV search
				if (score > alpha)
				{
					/* Once you've found a move with a score that is between alpha and beta,
					* the rest of the moves are searched with the goal of proving that they are all bad.
					* It's possible to do this a bit faster than a search that worries that one
					* of the remaining moves might be good
					*/

					score = -Negamax(-alpha - 1, -alpha, depth - 1);

					/* If the algorithm finds out that it was wrong, and that one of the
					* subsequent moves was better than the first PV move, it has to search again,
					* in the normal alpha-beta manner. This happens sometimes, and it's a waste of time,
					* but generally not often enough to counteract the savings gained from doing the
					* "bad move proof" search referred to earlier.
					*/

					if ((score > alpha) && (score < beta))
					{
						score = -Negamax(-beta, -alpha, depth - 1);
					}
				}
			}
		}
		// no late move reductions until iterative deepening reaches > 6
		// will help the engine to not miss trivial mate in 3's
		else
		{
			// full depth search
			if (movesSearched == 0)
			{
				// score current move
				score = -Negamax(-beta, -alpha, depth - 1);
			}
			else
			{
				// PV search
				if (score > alpha)
				{
					score = -Negamax(-alpha - 1, -alpha, depth - 1);
					if ((score > alpha) && (score < beta))
					{
						score = -Negamax(-beta, -alpha, depth - 1);
					}
				}
				else
				{
					score = -Negamax(-beta, -alpha, depth - 1);
				}
			}
		}

		

		// decrement ply
		mPly--;

		// take move back
		mModel.UnmakeMove(moves[count]);

		// increment number of moves searched
		movesSearched++;

		// fail-hard beta cutoff
		if (score >= beta)
		{
			// store transposition score with the BETA flag
			RecordHash(score, depth, HASH_FLAG_BETA);

			if (!decode_capture_flag(moves[count]))
			{
				// store the killer move
				mKillerMoves[1][mPly] = mKillerMoves[0][mPly];
				mKillerMoves[0][mPly] = moves[count];
			}
			// move fails high
			return beta;
		}

		if (score > alpha)
		{
			// switch to exact flag
			hashf = HASH_FLAG_EXACT;

			if (!decode_capture_flag(moves[count]))
			{
				if (mHistoryTable[decode_piece_type(moves[count])][decode_destination_square(moves[count])] < 7000)
				{
					// update the history table
					mHistoryTable[decode_piece_type(moves[count])][decode_destination_square(moves[count])] += depth;
				}
			}

			// PV move
			alpha = score;

			// write PV move
			mPvTable[mPly][mPly] = moves[count];

			// loop oever next ply
			for (int nextPly = mPly + 1; nextPly < mPvLength[mPly + 1]; nextPly++)
			{
				// copy move from deeper ply into a current ply's line
				mPvTable[mPly][nextPly] = mPvTable[mPly + 1][nextPly];
			}

			// adjust PV length
			mPvLength[mPly] = mPvLength[mPly + 1];
		}
	}


	// store the hash
	RecordHash(alpha, depth, hashf);

	// check if stalemate or checkmate
	if (legalMoves == 0)
	{
		if (inCheck)
		{
			// checkmate
			RecordHash(-49000, depth, HASH_FLAG_EXACT);
			return -49000 + mPly;
		}
		else
		{
			// stalemate
			RecordHash(0, depth, HASH_FLAG_EXACT);
			return 0;
		}
	}

	// move fails low
	return alpha;
}

int Agent::QuiescenceNegamax(int alpha, int beta, int depth)
{
	mNodes++;

	int score;
	if ((score = ProbeHash(alpha, beta, depth)) != NO_HASH_ENTRY)
	{
		// return the value if the position has been searched before
		//std::cout << "here" << std::endl;

		if (score == 49000)
			return score - mPly;
		if (score == -49000)
			return score + mPly;

		return score;
	}

	// evaluate position
	int standingPat = Evaluate();
	// fail-hard beta cutoff
	if (standingPat >= beta)
	{
		return beta;
	}

	if (standingPat > alpha)
	{
		alpha = standingPat;
	}

	

	// create move list instance
	std::vector<unsigned long long> moves;
	mModel.generateMoves(moves);

	// sort the moves
	OrderMoves(moves, mPly);

	for (int count = 0; count < moves.size(); count++)
	{

		// skip non-capture moves
		if (!decode_capture_flag(moves[count]) && !decode_promoted_piece_type(moves[count]))
		{
			continue;
		}


		// increment ply
		mPly++;

		// make the move
		mModel.MakeMove(moves[count], allMoves);

		// make sure to make only legal moves
		if (mModel.IsIllegal())
		{
			// take back the move
			mModel.UnmakeMove(moves[count]);
			mPly--;

			continue;
		}

		// score current move
		score = -QuiescenceNegamax(-beta, -alpha, depth - 1);

		// decremetn ply
		mPly--;

		// take move back
		mModel.UnmakeMove(moves[count]);



		// fail-hard beta cutoff
		if (score >= beta)
		{
			// move fails high
			return beta;
		}

		if (score > alpha)
		{
			// PV move
			alpha = score;
		}
	}

	// move fails low
	return alpha;
}


void Agent::InitializeKingBonusPassedPawnTable()
{

	mKingBonusPassedPawn.push_back({});
	mKingBonusPassedPawn.push_back({});

	int kingInFrontBonus = 65;

	// given a passed pawn square
	for (int square = 0; square < 64; square++)
	{
		// create a gradient field
		int rank = square / 8;
		int file = square % 8;
		mKingBonusPassedPawn[white].push_back({});
		mKingBonusPassedPawn[black].push_back({});
		std::vector <int> bonusTableWhite;
		std::vector <int> bonusTableBlack;

		for (int targetSquare = 0; targetSquare < 64; targetSquare++)
		{
			int targetRank = targetSquare / 8;
			int targetFile = targetSquare % 8;

			int distance;
			int whiteBonus = 0;
			int blackBonus = 0;


			// figure out the distance between the squares
			if (abs(rank - targetRank) > abs(file - targetFile))
			{
				distance = abs(rank - targetRank);
			}
			else
			{
				distance = abs(file - targetFile);
			}

			// invert
			distance = 8 - distance;



			// give a bonus for squares in front of the passed pawn square
			// or squares next to them
			if (targetFile - 1 == file)
			{
				if (targetSquare >= square + 2)
					whiteBonus += kingInFrontBonus;
				if (targetSquare <= square - 2)
					blackBonus += kingInFrontBonus;
			}

			if (targetFile + 1 == file)
			{
				if (targetSquare >= square + 2)
					whiteBonus += kingInFrontBonus;
				if (targetSquare <= square - 2)
					blackBonus += kingInFrontBonus;
			}

			if (targetFile == file)
			{
				if (targetSquare > square && targetRank != 7)
					whiteBonus += kingInFrontBonus;
				if (targetSquare < square && targetRank != 0)
					blackBonus += kingInFrontBonus;
			}

			// add calculated bonus to respective tables
			bonusTableWhite.push_back(distance + whiteBonus);
			bonusTableBlack.push_back(distance + blackBonus);
		}

		// add table to white/black + square
		mKingBonusPassedPawn[white][square] = bonusTableWhite;
		mKingBonusPassedPawn[black][square] = bonusTableBlack;
	}

}

void Agent::InitializeDistanceTables()
{
	int i, j;

	// set the size of the "square 1" dimension
	mDistanceTable.resize(64);
	mQueenDistanceTable.resize(64);
	mRookDistanceTable.resize(64);
	mKnightDistanceTable.resize(64);

	for (i = 0; i < 64; i++)
	{
		// set the size of the "square 2" dimension
		mDistanceTable[i].resize(64);
		mQueenDistanceTable[i].resize(64);
		mRookDistanceTable[i].resize(64);
		mKnightDistanceTable[i].resize(64);

		for (j = 0; j < 64; j++)
		{
			int COL_I = i % 8;
			int COL_J = j % 8;
			int ROW_I = i / 8;
			int ROW_J = j / 8;

			// set the respective distances to the formula on the wiki
			mDistanceTable[i][j] = 14 - (abs(COL_I - COL_J) + abs(ROW_I - ROW_J));
			mQueenDistanceTable[i][j] = (mDistanceTable[i][j] * 5) / 2;
			mRookDistanceTable[i][j] = mDistanceTable[i][j] / 2;
			mKnightDistanceTable[i][j] = mDistanceTable[i][j];
		}
	}
}



int Agent::Evaluate()
{
	//std::vector <int> debugWhite = {};
	//std::vector <int> debugBlack = {};

	// count the white pieces
	int whitePawnsCount = 0;
	int whiteKnightsCount = 0;
	int whiteBishopsCount = 0;
	int whiteRooksCount = 0;
	int whiteQueensCount = 0;
	int whiteKingCount = 0;

	// count the black pieces
	int blackPawnsCount = 0;
	int blackKnightsCount = 0;
	int blackBishopsCount = 0;
	int blackRooksCount = 0;
	int blackQueensCount = 0;
	int blackKingCount = 0;



	// initialize copies, totals, controlled squares
	unsigned long long bitboardCopy = 0;
	int whiteTotal = 0;
	int blackTotal = 0;
	int whiteControlledSquares = 0;
	int blackControlledSquares = 0;

	// figure out if white/black is castled king or queenside
	bool blackCastledShort = (mModel.mBitboards[k] & KINGSIDE_CASTLE_SQUARES) > 0;
	bool whiteCastledShort = (mModel.mBitboards[K] & KINGSIDE_CASTLE_SQUARES) > 0;

	bool blackCastledLong = (mModel.mBitboards[k] & QUEENSIDE_CASLTE_SQUARES) > 0;
	bool whiteCastledLong = (mModel.mBitboards[K] & QUEENSIDE_CASLTE_SQUARES) > 0;


	int whiteKingSquare = get_LS1B_index(mModel.mBitboards[K]);
	int blackKingSquare = get_LS1B_index(mModel.mBitboards[k]);
	unsigned long long whiteKingRing = mAttackTables.mKingAttacks[whiteKingSquare];
	unsigned long long blackKingRing = mAttackTables.mKingAttacks[blackKingSquare];

	unsigned long long attackBitboard = 0;

	int whitePassedPawnBonus = 0;
	int blackPassedPawnBonus = 0;

	int notDevelopedPenalty = 25;
	bool earlyGame = mModel.GetFullMoves() < 20;

	// init connected pawn variable
	bool lastPawnWasPassed = false;
	unsigned long long bishopXrayAttacks = 0;

	int rook1;
	int rook2;
	int rookSeventhRankCount = 0;
	int rookSecondRankCount = 0;

	bool pawnDoubled = false;
	unsigned long long isolatedBitboard = 0;

	// king saftey variables
	int whiteKingSafety = 0;
	int blackKingSafety = 0;

	int whiteKingShield = 0;
	int blackKingShield = 0;

	int tropismToWhiteKing = 0;
	int tropismToBlackKing = 0;



	// loop through the bitboards
	for (int i = 0; i < mModel.mBitboards.size(); i++)
	{
		if (i == K)
		{
			// reset passed pawn flag for black
			lastPawnWasPassed = false;
		}

		bitboardCopy = mModel.mBitboards[i];

		while (bitboardCopy)
		{

			int square = get_LS1B_index(bitboardCopy);
			int file = square % 8;
			int rank = square / 8;
			attackBitboard = 0;

			if (i == P)
			{
				whitePawnsCount++;
				whiteTotal += 100;
				whiteTotal += pawnWhitePlacementTable[square];

				// calculate distance from center file (get a bonus)
				whiteTotal += closenessToCenterFileBonus[square];

				// isolated pawns penalty

				// reset isolated bitboard
				isolatedBitboard = 0;

				if (file > 0)
				{
					isolatedBitboard |= fileConstants[file - 1];
				}
				if (file < 7)
				{
					isolatedBitboard |= fileConstants[file + 1];
				}

				if ((isolatedBitboard & mModel.mBitboards[P]) == 0)
				{
					// isolated pawn detected
					whiteTotal -= 9;
				}


				// double pawns penalty
				if ((mModel.mBitboards[P] & fileConstants[file]) != get_bit(mModel.mBitboards[P], square))
				{
					whiteTotal -= 3;

					// doubled side pawns are bad
					if (file == 0 || file == 7)
					{
						whiteTotal -= 20;
					}

					pawnDoubled = true;
				}
				else
				{
					pawnDoubled = false;
				}

				if ((mAttackTables.mPassedPawnMasks[white][square] & mModel.mBitboards[p]) == 0)
				{
					// passed pawn
					whitePassedPawnBonus += 30 * rank;

					// doubled pawns on the h/a files are really bad
					if (pawnDoubled && (file == 0 || file == 7))
					{
						// if the doubled pawn is behind, give a negative value
						// mask the file
						unsigned long long doubledFile = fileConstants[file] & mModel.mBitboards[P];

						// get the leading pawn square
						int pawnSquareInFront = 0;
						while (doubledFile)
						{
							pawnSquareInFront = get_LS1B_index(doubledFile);
							clear_bit(doubledFile, pawnSquareInFront);
						}

						if (pawnSquareInFront != square)
						{
							whitePassedPawnBonus -= rank * 35 + 100;
						}
						else
						{
							whitePassedPawnBonus -= rank * 25;
						}
					}

					// if the king is there to help, give a bonus
					whitePassedPawnBonus += mKingBonusPassedPawn[white][square][whiteKingSquare];
					blackPassedPawnBonus += mKingBonusPassedPawn[white][square][blackKingSquare];

					// opposition
					if ((whiteKingSquare % 8 == file - 1 && blackKingSquare % 8 == file + 1) || ((whiteKingSquare % 8 == file + 1 && blackKingSquare % 8 == file - 1)))
					{
						whitePassedPawnBonus += 100;
					}

					// connected passed pawn bonus
					if (lastPawnWasPassed)
					{
						whiteTotal += 50;
					}

					lastPawnWasPassed = true;
				}
				else
				{
					lastPawnWasPassed = false;
				}

				attackBitboard = mAttackTables.mPawnAttacks[white][square];

				// connected pawns
				whiteTotal += countBits((attackBitboard & mModel.mBitboards[P]));

			}
			else if (i == N)
			{
				whiteKnightsCount++;
				whiteTotal += 320;
				whiteTotal += knightPlacementTable[square];

				tropismToBlackKing += mKnightDistanceTable[square][blackKingSquare];

				// develop pieces early
				if (earlyGame && (square == b1 || square == g1))
				{
					whiteTotal -= notDevelopedPenalty;
				}

				attackBitboard = mAttackTables.mKnightAttacks[square];

				whiteControlledSquares += countBits(attackBitboard);

			}
			else if (i == B)
			{
				whiteBishopsCount++;
				whiteTotal += 330;
				whiteTotal += bishopPlacementTable[square];

				tropismToBlackKing += mRookDistanceTable[square][blackKingSquare];

				if (earlyGame && (square == c1 || square == f1))
				{
					whiteTotal -= notDevelopedPenalty;
				}

				// stop giving checks in the early game on b5, g5
				if (earlyGame)
				{
					if (square == b5 && get_bit(mModel.mOccupancies[black], c6))
					{
						whiteTotal += 2;
					}
					if (square == g5 && get_bit(mModel.mOccupancies[black], f6))
					{
						whiteTotal += 2;
					}
				}

				// don't block the center pawns with the bishop early
				if ((square == d3 && get_bit(mModel.mBitboards[P], d2)) || (square == e3 && get_bit(mModel.mBitboards[P], e2)))
				{
					whiteTotal -= 8;
				}

				// get all squares that a bishop go attack if there were no pieces
				bishopXrayAttacks = mAttackTables.getBishopAttacks(square, 0);

				// "biting on granite" penalty
				whiteTotal -= countBits(bishopXrayAttacks & mModel.mBitboards[p]);

				attackBitboard = mAttackTables.getBishopAttacks(square, mModel.mOccupancies[both]);

				whiteControlledSquares += countBits(attackBitboard);
			}
			else if (i == R)
			{
				whiteRooksCount++;
				whiteTotal += 500;
				whiteTotal += rookPlacementTable[square];

				tropismToBlackKing += mRookDistanceTable[square][blackKingSquare];

				// "seventh" rank (0 index)
				if (rank == 6)
				{
					whiteTotal += 20;
					rookSeventhRankCount++;
				}

				// controlling open files
				if ((mModel.mBitboards[R] & fileConstants[file]) == (mModel.mOccupancies[both] & fileConstants[file]))
				{
					whiteTotal += 5;
				}

				// trapped rook without castling
				if ((whiteKingSquare == f1 || whiteKingSquare == g1) && (square == g1 || square == h1))
				{
					whiteTotal -= 10;
				}



				// keep track of rook squares
				if (whiteRooksCount == 1)
				{
					rook1 = square;
				}
				if (whiteRooksCount == 2)
				{
					rook2 = square;
				}

				if (whiteRooksCount == 2)
				{
					// rook pair
					whiteTotal += 10;

					whiteTotal += ConnectedRooksBonus(rook1, rook2);

					if (rookSeventhRankCount >= 2)
					{
						whiteTotal += 6;
					}
				}

				attackBitboard = mAttackTables.getRookAttacks(square, mModel.mOccupancies[both]);


				// trapped rook
				if (countBits(attackBitboard) <= 3)
				{
					if (square == a1 && get_bit(mModel.mOccupancies[both], a3) || square == h1 && get_bit(mModel.mOccupancies[both], h3))
					{
						whiteTotal -= 105;
					}
					else
					{
						whiteTotal -= 20;
					}
				}

				whiteControlledSquares += countBits(attackBitboard);
			}
			else if (i == Q)
			{
				whiteQueensCount++;
				whiteTotal += 900;
				whiteTotal += queenPlacementTable[square];

				tropismToBlackKing += mQueenDistanceTable[square][blackKingSquare];

				// don't bring the queen out too early
				if (earlyGame && square > 24)
				{
					whiteTotal -= 10;
				}

				attackBitboard = mAttackTables.getQueenAttacks(square, mModel.mOccupancies[both]);

				whiteControlledSquares += countBits(attackBitboard);
			}

			else if (i == p)
			{
				blackPawnsCount++;
				blackTotal += 100;
				blackTotal += pawnBlackPlacementTable[square];

				// calculate distance from center file
				blackTotal += closenessToCenterFileBonus[square];

				// isolated pawns penalty
				isolatedBitboard = 0;

				if (file > 0)
				{
					isolatedBitboard |= fileConstants[file - 1];
				}
				if (file < 7)
				{
					isolatedBitboard |= fileConstants[file + 1];
				}

				if ((isolatedBitboard & mModel.mBitboards[p]) == 0)
				{
					// isolated pawn detected
					blackTotal -= 9;
				}

				// double pawns penalty
				if ((mModel.mBitboards[p] & fileConstants[file]) != get_bit(mModel.mBitboards[p], square))
				{
					blackTotal -= 3;

					// doubled side pawns are bad
					if (file == 0 || file == 7)
					{
						blackTotal -= 20;
					}

					pawnDoubled = true;
				}
				else
				{
					pawnDoubled = false;
				}

				if ((mAttackTables.mPassedPawnMasks[black][square] & mModel.mBitboards[P]) == 0)
				{

					// passed pawn
					blackPassedPawnBonus += (7 - rank) * 30;


					// doubled pawns on the h/a files are really bad
					if (pawnDoubled && (file == 0 || file == 7))
					{
						// if the doubled pawn is behind, give a negative value
						// mask the file
						unsigned long long doubledFile = fileConstants[file] & mModel.mBitboards[p];

						if (get_LS1B_index(doubledFile) != square)
						{
							blackPassedPawnBonus -= (7 - rank) * 35 + 100;
						}
						else
						{
							blackPassedPawnBonus -= (7 - rank) * 25;
						}
					}


					// if the king is there to help, give a bonus
					blackPassedPawnBonus += mKingBonusPassedPawn[black][square][blackKingSquare];
					whitePassedPawnBonus += mKingBonusPassedPawn[black][square][whiteKingSquare];

					// opposition
					if ((whiteKingSquare % 8 == file - 1 && blackKingSquare % 8 == file + 1) || ((whiteKingSquare % 8 == file + 1 && blackKingSquare % 8 == file - 1)))
					{
						blackPassedPawnBonus += 100;
					}

					// connected passed pawn bonus
					if (lastPawnWasPassed)
					{
						blackTotal += 50;
					}
					lastPawnWasPassed = true;


				}
				else
				{
					lastPawnWasPassed = false;
				}

				attackBitboard = mAttackTables.mPawnAttacks[black][square];

				// connected pawns
				blackTotal += countBits(attackBitboard & mModel.mBitboards[p]);

			}
			else if (i == n)
			{
				blackKnightsCount++;
				blackTotal += 320;
				blackTotal += knightPlacementTable[square];

				tropismToWhiteKing += mKnightDistanceTable[square][whiteKingSquare];

				// develop pieces early
				if (earlyGame && (square == b8 || square == g8))
				{
					blackTotal -= notDevelopedPenalty;
				}

				attackBitboard = mAttackTables.mKnightAttacks[square];

				blackControlledSquares += countBits(attackBitboard);
			}
			else if (i == b)
			{
				blackBishopsCount++;
				blackTotal += 330;
				blackTotal += bishopPlacementTable[square];

				tropismToWhiteKing += mRookDistanceTable[square][whiteKingSquare];

				if (earlyGame && (square == c8 || square == f8))
				{
					blackTotal -= notDevelopedPenalty;
				}

				// don't block the center pawns with the bishop early
				if ((square == d6 && get_bit(mModel.mBitboards[p], d7)) || (square == e6 && get_bit(mModel.mBitboards[p], e7)))
				{
					blackTotal -= 8;
				}

				// stop giving checks in the early game on b4, g4
				if (earlyGame)
				{
					if (square == b4 && get_bit(mModel.mOccupancies[white], c3))
					{
						blackTotal += 2;
					}
					if (square == g4 && get_bit(mModel.mOccupancies[white], f3))
					{
						blackTotal += 2;
					}
				}

				// get all squares that a bishop go attack if there were no pieces
				bishopXrayAttacks = mAttackTables.getBishopAttacks(square, 0);

				// "biting on granite" penalty
				blackTotal -= countBits(bishopXrayAttacks & mModel.mBitboards[P]);

				attackBitboard = mAttackTables.getBishopAttacks(square, mModel.mOccupancies[both]);

				blackControlledSquares += countBits(attackBitboard);
			}
			else if (i == r)
			{
				blackRooksCount++;
				blackTotal += 500;
				blackTotal += rookPlacementTable[square];

				tropismToWhiteKing += mRookDistanceTable[square][whiteKingSquare];

				// Second rank rook (0 based index)
				if (rank == 1)
				{
					blackTotal += 20;
					rookSecondRankCount++;
				}

				// controlling open files
				if ((mModel.mBitboards[r] & fileConstants[file]) == (mModel.mOccupancies[both] & fileConstants[file]))
				{
					blackTotal += 5;
				}

				// trapped rook from not castling
				if ((blackKingSquare == f8 || blackKingSquare == g8) && (square == g8 || square == h8))
				{
					blackTotal -= 10;
				}


				// keep track of 
				if (blackRooksCount == 1)
				{
					rook1 = square;
				}
				if (blackRooksCount == 2)
				{
					rook2 = square;
				}

				if (blackRooksCount == 2)
				{
					// rook pair
					blackTotal += 10;

					// connected rook
					blackTotal += ConnectedRooksBonus(rook1, rook2);

					if (rookSecondRankCount == 2)
					{
						blackTotal += 6;
					}
				}

				attackBitboard = mAttackTables.getRookAttacks(square, mModel.mOccupancies[both]);



				// trapped rook
				if (countBits(attackBitboard) <= 3)
				{
					if (square == a8 && get_bit(mModel.mOccupancies[both], a6) || square == h8 && get_bit(mModel.mOccupancies[both], h6))
					{
						blackTotal -= 105;
					}
					else
					{
						blackTotal -= 20;
					}
				}

				blackControlledSquares += countBits(attackBitboard);
			}
			else if (i == q)
			{
				blackQueensCount++;
				blackTotal += 900;
				blackTotal += queenPlacementTable[square];

				tropismToWhiteKing += mQueenDistanceTable[square][whiteKingSquare];

				// don't bring the queen out too early
				if (earlyGame && square <= 40)
				{
					blackTotal -= 10;
				}

				attackBitboard = mAttackTables.getQueenAttacks(square, mModel.mOccupancies[both]);

				blackControlledSquares += countBits(attackBitboard);
			}


			// incentivize pieces to be near kings
			if (get_bit(whiteKingRing, square))
			{
				whiteTotal += 1;
			}

			if (get_bit(blackKingRing, square))
			{
				blackTotal += 1;
			}

			if (i < p)
			{

				// controlling the middle
				whiteTotal += countBits(attackBitboard & MIDDLE_SQUARES);


			}
			else
			{
				// controlling the middle
				blackTotal += countBits(attackBitboard & MIDDLE_SQUARES);

			}

			clear_bit(bitboardCopy, square);

			/*
			// ----- debugging ----
			if (i < p)
			{
				debugWhite.push_back(whiteTotal);
			}
			else
			{
				debugBlack.push_back(blackTotal);
			}
			*/

		}
	}

	// count the non pawn material for both sides
	// scale it by a factor of 2
	int whiteNonPawnMateial = (3 * whiteKnightsCount + 3 * whiteBishopsCount + 5 * whiteRooksCount + 9 * whiteQueensCount) * 2;
	int blackNonPawnMaterial = (3 * blackKnightsCount + 3 * blackBishopsCount + 5 * blackRooksCount + 9 * blackQueensCount) * 2;
	int lowestNonPawnMaterial = whiteNonPawnMateial < blackNonPawnMaterial ? whiteNonPawnMateial : blackNonPawnMaterial;

	// drawn positions
	// low material and no rook vs king
	if (whitePawnsCount == 0 && blackPawnsCount == 0)
	{
		if ((whiteNonPawnMateial / 2 < 7 && whiteRooksCount != 1 && blackNonPawnMaterial == 0) || (blackNonPawnMaterial / 2 < 7 && blackRooksCount != 1 && whiteNonPawnMateial == 0))
		{
			return 0;
		}
	}
	// calculate endgame constant
	// value between 0 and 34, (0 = not endgame, 34 = fully in endgame)
	int endgamePhaseConstant = 62 - lowestNonPawnMaterial;

	if (endgamePhaseConstant <= 0)
	{
		endgamePhaseConstant = 0;
	}
	// we should be fully in the endgame if one player has <= 14 points of non pawn material
	if (endgamePhaseConstant >= 34)
	{
		endgamePhaseConstant = 34;
	}

	// apply endgame passed pawn bonus
	// if the opponent has powerful pieces, apply a reduced bonus
	if (blackRooksCount > 0 || blackQueensCount > 0)
	{
		whitePassedPawnBonus /= 3;
	}

	if (whiteRooksCount > 0 || whiteQueensCount > 0)
	{
		blackPassedPawnBonus /= 3;
	}

	if (whiteNonPawnMateial < blackNonPawnMaterial)
	{
		whitePassedPawnBonus /= 5;
	}

	if (blackNonPawnMaterial < whiteNonPawnMateial)
	{
		blackNonPawnMaterial /= 5;
	}

	whiteTotal += endgamePhaseConstant * whitePassedPawnBonus / 34;
	blackTotal += endgamePhaseConstant * blackPassedPawnBonus / 34;

	// apply king bonuses
	//white king
	int square = get_LS1B_index(mModel.mBitboards[K]);
	whiteTotal += 10000;
	// add the middle game and endgame values (tapered)
	whiteTotal += (34 - endgamePhaseConstant) * kingPlacementTable[square] / 34;
	whiteTotal += endgamePhaseConstant * kingEndgamePlacementTable[square] / 34;

	// if the king is exposed, apply a penalty
	whiteTotal -= (34 - endgamePhaseConstant) * countBits(mAttackTables.getQueenAttacks(square, mModel.mOccupancies[both])) / 34;


	square = get_LS1B_index(mModel.mBitboards[k]);
	blackTotal += 10000;
	// add the middle game and endgame values (tapered)
	blackTotal += (34 - endgamePhaseConstant) * kingPlacementTable[square] / 34;
	blackTotal += endgamePhaseConstant * kingEndgamePlacementTable[square] / 34;

	// if the king is exposed, apply a penalty
	blackTotal -= (34 - endgamePhaseConstant) * countBits(mAttackTables.getQueenAttacks(square, mModel.mOccupancies[both])) / 34;

	// king castle saftey
	if (whiteCastledShort)
	{
		if (get_bit(mModel.mBitboards[P], g2))
		{
			whiteTotal += 5;
		}
		else if (get_bit(mModel.mBitboards[P], g3))
		{
			whiteKingShield -= 1;
			whiteTotal += 2;
		}
		else
		{
			whiteKingShield -= 1;
			whiteTotal -= 10;
		}

		if (get_bit(mModel.mBitboards[P], f2))
		{
			whiteTotal += 6;
		}
		else
		{
			whiteKingShield -= 1;
			whiteTotal -= 10;
		}

		if ((blackQueensCount || blackRooksCount) && (mModel.mBitboards[P] & fileConstants[6]) == 0)
		{
			whiteKingShield -= 1;
			whiteTotal -= 75;
		}
	}

	if (whiteCastledLong)
	{
		if (get_bit(mModel.mBitboards[P], b2))
		{
			whiteTotal += 5;
		}
		else if (get_bit(mModel.mBitboards[P], b3))
		{
			whiteKingShield -= 1;
			whiteTotal += 2;
		}
		else
		{
			whiteKingShield -= 1;
			whiteTotal -= 10;
		}

		if (get_bit(mModel.mBitboards[P], c2))
		{
			whiteTotal += 6;
		}
		else
		{
			whiteKingShield -= 1;
			whiteTotal -= 10;
		}

		if ((blackQueensCount || blackRooksCount) && (mModel.mBitboards[P] & fileConstants[1]) == 0)
		{
			whiteKingShield -= 1;
			whiteTotal -= 75;
		}
	}

	if (blackCastledShort)
	{
		if (get_bit(mModel.mBitboards[p], g7))
		{
			blackTotal += 5;
		}
		else if (get_bit(mModel.mBitboards[p], g6))
		{
			blackKingShield -= 1;
			blackTotal += 2;
		}
		else
		{
			blackKingShield -= 1;
			blackTotal -= 10;
		}

		if (get_bit(mModel.mBitboards[p], f7))
		{
			blackTotal += 6;
		}
		else
		{
			blackKingShield -= 1;
			blackTotal -= 10;
		}

		if ((whiteQueensCount || whiteRooksCount) && (mModel.mBitboards[p] & fileConstants[6]) == 0)
		{
			blackKingShield -= 1;
			blackTotal -= 75;
		}
	}

	if (blackCastledLong)
	{
		if (get_bit(mModel.mBitboards[p], b7))
		{
			blackTotal += 5;
		}
		else if (get_bit(mModel.mBitboards[p], b6))
		{
			blackKingShield -= 1;
			blackTotal += 2;
		}
		else
		{
			blackKingShield -= 1;
			blackTotal -= 10;
		}

		if (get_bit(mModel.mBitboards[p], c7))
		{
			blackTotal += 6;
		}
		else
		{
			blackKingShield -= 1;
			blackTotal -= 10;
		}

		if ((whiteQueensCount || whiteRooksCount) && (mModel.mBitboards[p] & fileConstants[1]) == 0)
		{
			blackKingShield -= 1;
			blackTotal -= 75;
		}
	}

	int notCastledPenalty = 0;
	// really late castling pentalty
	if (!whiteCastledLong && !whiteCastledShort && mModel.mFullMoves < 20)
	{
		notCastledPenalty = ((34 - endgamePhaseConstant) * 5 * mModel.mFullMoves) / 34;
		whiteTotal -= notCastledPenalty;
	}
	if (!blackCastledLong && !blackCastledShort && mModel.mFullMoves < 20)
	{
		notCastledPenalty = ((34 - endgamePhaseConstant) * 5 * mModel.mFullMoves) / 34;
		blackTotal -= notCastledPenalty;
	}

	// two minor pieces are better than a rook
	if (whiteRooksCount - blackRooksCount == 1 &&
		(whiteBishopsCount + whiteKnightsCount) - (blackBishopsCount + blackKnightsCount) == 2 &&
		blackQueensCount == whiteQueensCount)
	{
		whiteTotal -= 100;
	}

	if (blackRooksCount - whiteRooksCount == 1 &&
		(blackBishopsCount + blackKnightsCount) - (whiteBishopsCount + whiteKnightsCount) == 2 &&
		blackQueensCount == whiteQueensCount)
	{
		blackTotal -= 100;
	}

	// give a bonus for bishop pair
	if (whiteBishopsCount >= 2)
	{
		whiteTotal += 20;
	}

	if (blackBishopsCount >= 2)
	{
		blackTotal += 20;
	}

	whiteTotal += whiteControlledSquares;
	blackTotal += blackControlledSquares;

	// tempo, give a bonus for having the right to move

	if (mModel.mSideToMove == white)
	{
		whiteTotal += 10;
	}
	else
	{
		blackTotal += 10;
	}

	// overall king safety scores

	whiteKingSafety = ((whiteKingShield - tropismToWhiteKing) * blackNonPawnMaterial) / INITIAL_PIECE_MATERIAL;
	blackKingSafety = ((blackKingShield - tropismToBlackKing) * whiteNonPawnMateial) / INITIAL_PIECE_MATERIAL;

	//std::cout << "WKS: " << whiteKingSafety << std::endl;
	//std::cout << "BKS " << blackKingSafety << std::endl;

	whiteTotal += whiteKingSafety;
	blackTotal += blackKingSafety;

	int eval = whiteTotal - blackTotal;

	// force the king to the side of the board in the endgame
	if (mModel.GetSideToMove() == white)
	{
		eval += endgamePhaseConstant * ForceKingToCorner(whiteKingSquare, blackKingSquare) / 34;
	}
	else
	{
		eval -= endgamePhaseConstant * ForceKingToCorner(blackKingSquare, whiteKingSquare) / 34;
	}

	// negamax implementation
	if (mModel.mSideToMove == white)
	{
		return eval;
	}
	else
	{
		return -eval;
	}
}



int Agent::ForceKingToCorner(int friendlyKingSquare, int opponentKingSquare)
{

	int eval = 0;
	// get the opponent rank and file
	int opponentKingRank = opponentKingSquare / 8;
	int opponentKingFile = opponentKingSquare % 8;

	// get friendly rank and file
	int friendlyKingRank = friendlyKingSquare / 8;
	int friendlyKingFile = friendlyKingSquare % 8;

	int opponentKingDstToCenterFile = std::max(3 - opponentKingFile, opponentKingFile - 4);
	int opponentKingDstToCenterRank = std::max(3 - opponentKingRank, opponentKingRank - 4);
	int opponentKingDstToCenter = opponentKingDstToCenterFile + opponentKingDstToCenterRank;
	eval += opponentKingDstToCenter;

	int dstBetweenKingsFile = std::abs(friendlyKingFile - opponentKingFile);
	int dstBetweenKingsRank = std::abs(friendlyKingRank - opponentKingRank);
	int dstBetweenKings = dstBetweenKingsFile + dstBetweenKingsRank;
	eval += (28 - 2 * dstBetweenKings);

	return eval;
}


int Agent::ConnectedRooksBonus(int square1, int square2)
{
	// check if rooks are on same rank
	if (square1 / 8 == square2 / 8)
	{
		int leftRook;
		int rightRook;
		if (square1 < square2)
		{
			leftRook = square1;
			rightRook = square2;
		}
		else
		{
			leftRook = square2;
			rightRook = square1;
		}
		// now loop through the squares between them and see if there is nothing in the way
		for (int i = leftRook + 1; i < rightRook - 1; i++)
		{
			if (get_bit(mModel.mOccupancies[both], i))
			{
				return 0;
			}
		}
		// no pieces in between
		return 4;
	}

	// check if the rooks are on the same file
	if (square1 % 8 == square2 % 8)
	{
		int topRook;
		int bottomRook;
		if (square1 > square2)
		{
			topRook = square1;
			bottomRook = square2;
		}
		else
		{
			topRook = square2;
			bottomRook = square1;
		}

		for (int i = bottomRook + 8; i < topRook - 8; i += 8)
		{
			if (get_bit(mModel.mOccupancies[both], i))
			{
				return 0;
			}
		}

		// no pieces in the way
		return 4;
	}
	return 0;
}
