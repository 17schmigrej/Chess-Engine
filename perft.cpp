#include "perft.h"
#include "helper.h"

long perftTest(Model* startPosition, Zobrist* zTables, int depth, bool info)
{
	long count = 0;
	long captures = 0;
	long enpassant = 0;
	long castles = 0;
	//std::cout << castles << std::endl;
	unsigned long long time_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	perftTestRecursive(startPosition, zTables, depth, count, captures, enpassant, castles);
	unsigned long long time_end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	if (info)
	{
		std::cout << "Time taken (ms):" << time_end - time_start << std::endl;
		std::cout << "captures\tep\tcastles" << std::endl;
		std::cout << captures << "\t\t" << enpassant << "\t" << castles << std::endl;
	}
	
	return count;
}

void perftTestRecursive(Model* position, Zobrist* zTables, int depth, long &count, long &captures, long &enpassant, long &castles)
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
		position->generateMoves(moves);

		// loop over moves and recursively count their possible positions
		for (int i = 0; i < moves.size(); i++)
		{
			position->MakeMove(moves[i], allMoves);
			if (position->IsIllegal())
			{
				//std::cout << "illegal" << std::endl;
				//position->PrintBoard();
				//printMoveInformation(moves[i]);
				position->UnmakeMove(moves[i]);
				continue;
			}
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

			perftTestRecursive(position, zTables, depth - 1, count, captures, enpassant, castles);
		}

	}
}

void perftEachMove(Model* startPosition, Zobrist* zTables, int depth)
{
	std::vector <unsigned long long> moves;

	startPosition->generateMoves(moves);
	long count = 0;
	printMoveList(moves);
	unsigned long long time_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < moves.size(); i++)
	{
		std::vector <unsigned long long> newMoves;


		startPosition->MakeMove(moves[i], allMoves);
		if (startPosition->IsIllegal())
		{
			continue;
		}
		printMove(moves[i]);
		std::cout << ": ";
		long positionCount = perftTest(startPosition, zTables, depth, false);
		count += positionCount;
		std::cout << positionCount << std::endl;
	}
	unsigned long long time_end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::cout << "\nTotal time: " << time_end - time_start << std::endl;;
	std::cout << "\nTotal moves: " << count << std::endl;
}
