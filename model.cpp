#include "model.h"

Model::Model(AttackTables* attackTable, Zobrist* zTables)
{
	mIllegalState = 0;
	mBitboards.resize(12);
	mOccupancies.resize(3);
	mSideToMove = white;
	mEnPassant = noSquare;
	mCastleRights = 15;
	mHalfMoveClock = 0;
	mFullMoves = 0;
	mHash = 0;

	mOldEnpassant = {};
	mOldCastleRights = {};
	mOldHalfMoveClock = {};
	mOldHash = {};
	mLastPieceCaptured = {};
	
	mBitboards[P] = 0xff00;
	mBitboards[R] = 0x81;
	mBitboards[N] = 0x42;
	mBitboards[B] = 0x24;
	mBitboards[Q] = 0x8;
	mBitboards[K] = 0x10;

	mBitboards[p] = 0xff000000000000;
	mBitboards[r] = 0x8100000000000000;
	mBitboards[n] = 0x4200000000000000;
	mBitboards[b] = 0x2400000000000000;
	mBitboards[q] = 0x800000000000000;
	mBitboards[k] = 0x1000000000000000;

	mAttackTables = attackTable;
	mZobristTables = zTables;

	
	UpdateOccupancies();
}


Model::Model(AttackTables* attackTable, Zobrist* zTables, int side, int enpassant, int castle, int halfmove, int fullmove, std::vector<unsigned long long> const& bitboards, std::vector<unsigned long long>& occ, unsigned long long hash)
{
	mIllegalState = 0;
	mBitboards.resize(12);
	mOccupancies.resize(3);
	mSideToMove = side;
	mEnPassant = enpassant;
	mCastleRights = castle;
	mHalfMoveClock = halfmove;
	mFullMoves = fullmove;
	mHash = hash;

	mOldEnpassant = {};
	mOldCastleRights = {};
	mOldHalfMoveClock = {};
	mOldHash = {};
	mLastPieceCaptured = {};

	for (int i = 0; i < bitboards.size(); i++)
	{
		mBitboards[i] = bitboards[i];
	}

	for (int i = 0; i < occ.size(); i++)
	{
		mOccupancies[i] = occ[i];
	}

	mAttackTables = attackTable;
	mZobristTables = zTables;
}

Model::~Model()
{
}

void Model::SetBitboards(std::vector<unsigned long long> bbs)
{
	for (int i = 0; i < bbs.size(); i++)
	{
		mBitboards[i] = bbs[i];
	}
}

void Model::SetOccupancies(std::vector<unsigned long long> occ)
{
	for (int i = 0; i < mOccupancies.size(); i++)
	{
		mOccupancies[i] = occ[i];
	}
}

void Model::SetSideToMove(int stm)
{
	mSideToMove = stm;
}

void Model::SetEnpassant(int ep)
{
	mEnPassant = ep;
}

void Model::SetCastlingRights(int castle)
{
	mCastleRights = castle;
}

void Model::SetHalfMoveClock(int hmc)
{
	mHalfMoveClock = hmc;
}

void Model::SetFullMoves(int fm)
{
	mFullMoves = fm;
}

void Model::SetHash(unsigned long long hash)
{
	mHash = hash;
}

void Model::LoadPosition(std::string FEN, Zobrist * zTables)
{
	// reset position
	for (int i = 0; i < pieceLast; i++)
	{
		mBitboards[i] = 0x0;
	}
	mCastleRights = 0;

	// start parsing FEN

	// this will help us keep track of sections
	int field = 0;
	int file = 0;
	int rank = 7;
	std::string halfMove = "";
	std::string fullMove = "";
	for (int c = 0; c < FEN.size(); c++)
	{
		int square = rank * 8 + file;
		if (field == 0)
		{
			// loading the pieces

			//check whether to skip or place a piece
			if (isalpha(FEN[c]))
			{
				;
				set_bit(mBitboards[asciiToConstant.at(FEN[c])], square);
				file++;
			}
			else if (isdigit(FEN[c]))
			{
				file += FEN[c] - '0';
			}
			else if (FEN[c] == '/')
			{
				rank--;
				file = 0;
			}
			else if (FEN[c] == ' ')
			{
				field++;
			}
			else
			{
				std::cout << "something isn't right 1" << std::endl;
			}
		}
		else if (field == 1)
		{
			// side to move field
			if (FEN[c] == 'w')
			{
				mSideToMove = white;
			}
			else if (FEN[c] == 'b')
			{
				mSideToMove = black;
			}
			else if (FEN[c] == ' ')
			{
				field++;
			}
			else
			{
				std::cout << "something isn't right 2" << std::endl;
			}
		}
		else if (field == 2)
		{
			// castling rights
			if (FEN[c] == '-')
			{
				// do nothing
			}
			else if (FEN[c] == 'K')
			{
				mCastleRights |= wk;
			}
			else if (FEN[c] == 'Q')
			{
				mCastleRights |= wq;
			}
			else if (FEN[c] == 'k')
			{
				mCastleRights |= bk;
			}
			else if (FEN[c] == 'q')
			{
				mCastleRights |= bq;
			}
			else if (FEN[c] == ' ')
			{
				field++;
			}
			else
			{
				std::cout << "something isn't right 3" << std::endl;
			}
		}
		else if (field == 3)
		{
			// enpassant
			if (FEN[c] == '-')
			{
				mEnPassant = noSquare;
			}
			else if (FEN[c] == ' ')
			{
				field++;
			}
			else if (isalnum(FEN[c]))
			{
				int enPassantFile = FEN[c] - 'a';
				int enPassantRank = FEN[c + 1] - '0' - 1;
				mEnPassant = enPassantRank * 8 + enPassantFile;
				c += 1;
			}
		}
		else if (field == 4)
		{
			// helf move clock
			if (isdigit(FEN[c]))
			{
				halfMove.push_back(FEN[c]);
			}
			else if (FEN[c] == ' ')
			{
				if (halfMove != "")
				{
					mHalfMoveClock = std::stoi(halfMove);
				}
				else
				{
					mHalfMoveClock = 0;
				}
				field++;
			}
		}
		else if (field == 5)
		{
			// full move clock
			if (isdigit(FEN[c]))
			{
				fullMove.push_back(FEN[c]);
			}
			else
			{
				break;
				std::cout << "something isn't right 4" << std::endl;
			}
		}
	}
	mFullMoves = std::stoi(fullMove);

	UpdateOccupancies();

	// initialize the hash key
	mHash = zTables->HashBoard(this);
}

void Model::PrintBoard()
{
	std::cout << "Printing Board \n" << std::endl;
	// loop by rank
	for (int rank = 0; rank < 8; rank++)
	{
		std::cout << "   " << 8 - rank << "    ";
		//loop file
		for (int file = 0; file < 8; file++)
		{
			int square_index = (7 - rank) * 8 + file;

			// print out the respective piece
			int pieceFound = 0;
			for (int i = 0; i < pieceLast; i++)
			{
				if (get_bit(mBitboards[i], square_index))
				{
					std::cout << asciiPieces[i] << " ";
					pieceFound = 1;
				}
			}
			if (!pieceFound)
			{
				std::cout << ". ";
			}
			
		}
		std::cout << std::endl;
	} 
	std::cout << "\n\tA B C D E F G H\n" << std::endl;


	std::cout << "    Side:\t";

	// side to move
	if (mSideToMove)
	{
		std::cout << "Black" << std::endl;
	}
	else
	{
		std::cout << "White" << std::endl;
	}

	// if en passant is available
	if (mEnPassant != noSquare)
	{
		std::cout << "    En passant:\t" << square_to_coordinates[mEnPassant] << std::endl;
	}

	// castling rights
	std::cout << "    Castling:\t";
	if (mCastleRights & wk)
	{
		std::cout << "K";
	}
	else
	{
		std::cout << "-";
	}
	if (mCastleRights & wq)
	{
		std::cout << "Q";
	}
	else
	{
		std::cout << "-";
	}
	if (mCastleRights & bk)
	{
		std::cout << "k";
	}
	else
	{
		std::cout << "-";
	}
	if (mCastleRights & bq)
	{
		std::cout << "q";
	}
	else
	{
		std::cout << "-";
	}
	std::cout << "\nHash: " << std::hex << mHash << std::dec << std::endl;

	std::cout << "End Printing Board" << std::endl;

}

void Model::PrintBitboard(int piece)
{
	printBitboard(mBitboards[piece]);
}

void Model::PrintAllBitboards()
{
	for (int i = 0; i < mBitboards.size(); i++)
	{
		std::cout << "Piece: " << asciiPieces[i] << std::endl;
		PrintBitboard(i);
	}

	std::cout << "White occupancy" << std::endl;
	printBitboard(mOccupancies[white]);

	std::cout << "Black occupancy" << std::endl;
	printBitboard(mOccupancies[black]);

	std::cout << "Both occupancy" << std::endl;
	printBitboard(mOccupancies[both]);


}

void Model::PrintOccupancy(int side)
{
	printBitboard(mOccupancies[side]);
}

void Model::PrintAttackedSquares(int side)
{
	for (int rank = 0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++)
		{
			int square = (7 - rank) * 8 + file;

			if (!file)
			{
				std::cout << "   " << 8 - rank << "    ";
			}

			// check whether current square is attacked or not
			std::cout << mAttackTables->SquareAttacked(square, side, mBitboards, mOccupancies) << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "\n\tA B C D E F G H\n" << std::endl;
}

bool Model::IsIllegal()
{
	return mIllegalState;
}

bool Model::KingCaptured()
{
	return mBitboards[K] == 0 || mBitboards[k] == 0;
}

int Model::Done(Zobrist * zTables)
{

	// generate the list of moves
	std::vector<unsigned long long> moves;
	generateMoves(moves);

	// get indices of legal moves
	std::vector<int> indices;
	for (int i = 0; i < moves.size(); i++)
	{
		MakeMove(moves[i], allMoves);
		if (IsIllegal())
		{
			continue;
		}
		indices.push_back(i);
	}

	// check for 50 move draw
	if (GetHalfMoveClock() >= 100)
	{
		//std::cout << "50 move rule draw" << std::endl;
		return 2;
	}


	return indices.size() == 0;
}

int Model::CheckDrawOrCheckmateOrInPlay(Zobrist * zTables)
{
	// generate the moves 
	std::vector <unsigned long long> moves;
	generateMoves(moves);

	// check if king has a legal move
	for (int i = 0; i < moves.size(); i++)
	{
		MakeMove(moves[i], allMoves);

		if (!IsIllegal())
		{
			UnmakeMove(moves[i]);
			if (mHalfMoveClock >= 100)
			{
				return draw;
			}
			else
			{
				return inplay;
			}
		}
		UnmakeMove(moves[i]);
	}

	// if the king has no legal moves, determine if the king is in CHECK
	int kingIndex = mSideToMove == white ? K : k;
	int kingSquare = get_LS1B_index(mBitboards[kingIndex]);
	int attackingSide = mSideToMove == white ? black : white;

	if (mAttackTables->SquareAttacked(kingSquare, attackingSide, mBitboards, mOccupancies))
	{
		// king is attacked
		return checkmate;
	}
	else
	{
		// king is not attacked
		return draw;
	}
}

long Model::perftTest(int depth, bool info)
{
	long count = 0;
	long captures = 0;
	long enpassant = 0;
	long castles = 0;
	//std::cout << castles << std::endl;
	unsigned long long time_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	perftTestRecursive(depth, count, captures, enpassant, castles);
	unsigned long long time_end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	if (info)
	{
		std::cout << "Time taken (ms):" << time_end - time_start << std::endl;
		std::cout << "captures\tep\tcastles" << std::endl;
		std::cout << captures << "\t\t" << enpassant << "\t" << castles << std::endl;
	}

	return count;
}

void Model::perftTestRecursive(int depth, long& count, long& captures, long& enpassant, long& castles)
{
	if (depth == 0)
	{
		count++;
		return;
	}
	else
	{
		// make a move list and fill it with moves from the given position
		std::vector <unsigned long long> moves;
		generateMoves(moves);

		// loop over moves and recursively count their possible positions
		for (int i = 0; i < moves.size(); i++)
		{
			MakeMove(moves[i], allMoves);
			if (IsIllegal())
			{
				UnmakeMove(moves[i]);
				//std::cout << "illegal" << std::endl;
				//position->PrintBoard();
				//printMoveInformation(moves[i]);
				continue;
			}
			//printMove(moves[i]);
			//std::cout << "----------------------------------------------------" << std::endl;
			//PrintBoard();
			if (decode_capture_flag(moves[i]))
			{
				captures++;
			}
			if (decode_enpassant_flag(moves[i]))
			{
				enpassant++;
			}
			if (decode_castling_flag(moves[i]))
			{
				castles++;
			}

			/*
			// hashing debugging
			// build a hash from scratch
			unsigned long long hashBuiltFromScratch = zTables->HashBoard(&newPosition);



			if (hashBuiltFromScratch != newPosition.GetHash())
			{
				printMove(moves[i]);
				std::cout << std::endl;
				newPosition.PrintBoard();
				std::cout << "Built-from-scratch hash: " << std::hex << hashBuiltFromScratch << std::dec << std::endl;
				getchar();
			}
			*/

			perftTestRecursive(depth - 1, count, captures, enpassant, castles);
			UnmakeMove(moves[i]);
		}

	}
}

void Model::perftEachMove(int depth)
{
	std::vector <unsigned long long> moves;

	generateMoves(moves);
	long count = 0;
	printMoveList(moves);
	unsigned long long time_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < moves.size(); i++)
	{
		//std::cout << "*************Beginning*******************" << std::endl;
		std::vector <unsigned long long> newMoves;


		MakeMove(moves[i], allMoves);
		if (IsIllegal())
		{
			UnmakeMove(moves[i]);
			continue;
		}
		//printMove(moves[i]);
		//std::cout << "----------------------------------------------------" << std::endl;
		//PrintBoard();
		//getchar();

		printMove(moves[i]);
		std::cout << ": ";
		long positionCount = perftTest(depth, false);
		UnmakeMove(moves[i]);
		count += positionCount;
		std::cout << positionCount << std::endl;
	}
	unsigned long long time_end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::cout << "\nTotal time: " << time_end - time_start << std::endl;;
	std::cout << "\nTotal moves: " << count << std::endl;
}

bool Model::OnlyPawns()
{
	// if either player only has pawns or nothing
	return ((!mBitboards[N] && !mBitboards[B] && !mBitboards[R] && !mBitboards[Q]) ||
			(!mBitboards[n] && !mBitboards[b] && !mBitboards[r] && !mBitboards[q]));
}


