#include "environment.h"

Environment::Environment(Zobrist* zTables) : mModel(&mAttackTables, zTables)
{
	//mModel = new Model();
	mMovesMade = {};
	mHashRepititionList = {};
	mZobristTables = zTables;
	mModel.SetHash(mZobristTables->HashBoard(&mModel));

}

Environment::~Environment()
{
	//delete mModel;
}

void Environment::PrintBoard()
{
	mModel.PrintBoard();
}

void Environment::PrintBitboard(int piece)
{
	mModel.PrintBitboard(piece);
}

void Environment::PrintOccupancy(int side)
{
	mModel.PrintOccupancy(side);
}

void Environment::LoadPosition(std::string FEN)
{
	mModel.LoadPosition(FEN, mZobristTables);
	mMovesMade = {};
	mHashRepititionList = {mModel.GetHash()};
	mMoveHashesHistory.clear();
}

Percepts Environment::GetPercepts()
{
	// create a state which is a copy of the model and return it
	Percepts state;

	state.bitboards = mModel.GetBitboards();
	state.occupancies = mModel.GetOccupancies();
	state.sideToMove = mModel.GetSideToMove();
	state.enPassant = mModel.GetEnpassant();
	state.castlingRights = mModel.GetCastlingRights();
	state.halfMoveClock = mModel.GetHalfMoveClock();
	state.fullMoves = mModel.GetFullMoves();
	state.hash = mModel.GetHash();

	for (const auto& i : mMoveHashesHistory)
	{
		state.movesHistory[i.first] = 1;
	}

	return state;
}

unsigned long long Environment::ParseMove(const char* moveString)
{
	// generate moves
	std::vector<unsigned long long> moves;
	mModel.generateMoves(moves);

	// get the source square
	int sourceSquare = (moveString[0] - 'a') + ((moveString[1] - '0') - 1) * 8;

	// get the destination square
	int destinationSquare = (moveString[2] - 'a') + ((moveString[3] - '0') - 1) * 8;



	for (int i = 0; i < moves.size(); i++)
	{
		unsigned long long move = moves[i];

		// make sure the source and destination squares are in the generated moves
		if (sourceSquare == decode_start_square(moves[i]) && destinationSquare == decode_destination_square(moves[i]))
		{
			// initialize promoted piece
			int promotedPieceType = decode_promoted_piece_type(moves[i]);

			// promoted piece flag is active
			if (promotedPieceType)
			{
				if ((promotedPieceType == Q || promotedPieceType == q) && moveString[4] == 'q')
				{
					// return legal move
					return move;
				}
				else if ((promotedPieceType == R || promotedPieceType == r) && moveString[4] == 'r')
				{
					// return legal move
					return move;
				}
				else if ((promotedPieceType == B || promotedPieceType == b) && moveString[4] == 'b')
				{
					// return legal move
					return move;
				}
				else if ((promotedPieceType == N || promotedPieceType == n) && moveString[4] == 'n')
				{
					// return legal move
					return move;
				}
				
				continue;
			}

			return move;
		}
	}

	// illegal move
	return 0;
}

void Environment::MakeMove(unsigned long long move)
{
	mMovesMade.push_back(move);
	mModel.MakeMove(move, allMoves);
	mHashRepititionList.push_back(mModel.GetHash());
	mMoveHashesHistory[mModel.GetHash()] = 1;
}

void Environment::ParsePosition(const char* command)
{
	// shift pointer to the right to when it says "fen" or "startpos"
	command += 9;

	// initialize pointer to the current character in the string
	const char* currentChar = command;

	if (strncmp(command, "startpos", 8) == 0)
	{
		// reset everything
		mMovesMade = {};
		mHashRepititionList = {};
		mMoveHashesHistory.clear();

		Model a(&mAttackTables, mZobristTables);
		mModel = a;
		mModel.SetHash(mZobristTables->HashBoard(&mModel));

	}
	else
	{
		// make sure "fen" command is in the string
		currentChar = strstr(command, "fen");

		// if no "fen" command
		if (currentChar == NULL)
		{
			// reset everything
			mMovesMade = {};
			mHashRepititionList = {};
			mMoveHashesHistory.clear();

			Model a(&mAttackTables, mZobristTables);
			mModel = a;
			mModel.SetHash(mZobristTables->HashBoard(&mModel));
		}
		else
		{
			// shift pointer to the right to where the fen starts
			currentChar += 4;

			// reset
			mMovesMade = {};
			mHashRepititionList = {};
			mMoveHashesHistory.clear();

			LoadPosition(currentChar);
		}
	}

	// parse moves after position loading
	currentChar = strstr(command, "moves");

	if (currentChar != NULL)
	{
		// shift token to the right
		currentChar += 6;
		std::cout << currentChar << std::endl;

		// loop oever moves in string
		while (*currentChar)
		{
			// parse next move
			unsigned long long move = ParseMove(currentChar);

			// if no more moves
			if (move == 0)
			{
				break;
			}

			// make move on the board
			MakeMove(move);

			// move the the pointer to the beginning of the next move
			while (*currentChar && *currentChar != ' ')
			{
				currentChar++;
			}

			// go to next move
			currentChar++;
		}

	}
}

int Environment::ParseGoDepth(const char* command)
{
	// initialize depth
	int depth = -1;

	// init char pointer to depth argument
	const char* currentDepth = NULL;

	// handle fixed depth search
	if (currentDepth = strstr(command, "depth"))
	{
		// convert str to integer and assign to depth
		depth = atoi(currentDepth + 6);
	}
	else
	{
		depth = 7;
	}

	std::cout << "depth: " << depth << std::endl;
	return depth;
}

bool Environment::Done()
{
	int endCondition = mModel.CheckDrawOrCheckmateOrInPlay(mZobristTables);
	if (endCondition == draw)
	{
		std::cout << "It's a draw by 50 move rule or stalemate" << std::endl;
		return true;
	}

	// check for 3-fold repitition
	if (mMovesMade.size() > 0)
	{
		int count = 0;
		unsigned long long lastMove = mHashRepititionList.back();
		for (int i = 0; i < mHashRepititionList.size(); i++)
		{
			if (mHashRepititionList[i] == lastMove)
			{
				count++;
			}
		}
		if (count >= 3)
		{
			std::cout << "3-fold repitition" << std::endl;
			return true;
		}
	}

	if (endCondition == checkmate)
	{
		std::cout << "King has no moves (checkmate)" << std::endl;
		return true;
	}
	else
	{
		return false;
	}
}

void Environment::PrintMadeMoves()
{
	for (int i = 0; i < mMovesMade.size(); i++)
	{
		printMove(mMovesMade[i]);
		std::cout << "\n";
	}
}

void Environment::ListCurrentPositionMoves()
{
	std::vector<unsigned long long> moves;
	mModel.generateMoves(moves);

	for (int i = 0; i < moves.size(); i++)
	{
		int startSquare = decode_start_square(moves[i]);
		int endSquare = decode_destination_square(moves[i]);
		std::cout << i << ". " << square_to_coordinates[startSquare] << square_to_coordinates[endSquare] << std::endl;
	}
}

void Environment::UndoMove()
{
	// get the last move played
	unsigned long long lastMovePlayed = mMovesMade[mMovesMade.size() - 1];
	unsigned long long hash = mModel.GetHash();
	mModel.UnmakeMove(lastMovePlayed);
	mMovesMade.pop_back();
	mHashRepititionList.pop_back();

	// if the position hasn't been reached before, remove it from the hash history map
	// find if the hash existed before
	bool hashExists = false;
	for (int i = 0; i < mHashRepititionList.size(); i++)
	{
		if (mHashRepititionList[i] == hash)
		{
			return;
		}
	}

	// hash doesn't exist
	mMoveHashesHistory.erase(hash);

}

