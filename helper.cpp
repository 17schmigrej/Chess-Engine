#include "helper.h"


void printBitboard(unsigned long long bitboard)
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

			// print 1 if bit is 1 at position square_index
			std::cout << (get_bit(bitboard, square_index) ? 1 : 0) << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "\n\tA B C D E F C H" << std::endl;
	std::cout << "\t  Bitboard: 0x" << std::hex << bitboard << std::dec << std::endl;
	std::cout << "\nEnd Printing Board" << std::endl;
}



unsigned int getRandom32BitNumber()
{
	// start with the beginning state
	static unsigned int randomState = 1804289383;

	// pseudo random number state
	unsigned int number = randomState;

	// XOR shift algorithm
	number ^= number << 13;
	number ^= number >> 17;
	number ^= number << 5;

	randomState = number;

	return number;
}

unsigned long long getRandom64BitNumber()
{
	// initialize 4 random numbers
	unsigned long long n1, n2, n3, n4;

	// only keep the 16 least significant bits
	n1 = (unsigned long long)(getRandom32BitNumber()) & 0xFFFF;
	n2 = (unsigned long long)(getRandom32BitNumber()) & 0xFFFF;
	n3 = (unsigned long long)(getRandom32BitNumber()) & 0xFFFF;
	n4 = (unsigned long long)(getRandom32BitNumber()) & 0xFFFF;

	// return random number
	return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}


unsigned long long generateMagicNumber()
{
	return getRandom64BitNumber() & getRandom64BitNumber() & getRandom64BitNumber();
}

void printMoveInformation(unsigned long long move)
{
	int piece_type = decode_piece_type(move);

	bool capture_flag = decode_capture_flag(move);
	bool enpassant_flag = decode_enpassant_flag(move);
	bool double_pawn_push_flag = decode_double_push_flag(move);
	bool castling_flag = decode_castling_flag(move);

	std::cout << "\nmove: ";
	printMove(move);
	std::cout << "\npiece type: " << asciiPieces[piece_type];
	std::cout << "\ncapture flag " << capture_flag << "\ndouble pawn push flag: " << double_pawn_push_flag;
	std::cout << "\nenpassant flag: " << enpassant_flag << "\ncastling flag: " << castling_flag << std::endl << std::endl;;
}

void printMove(unsigned long long move)
{
	int start_square = decode_start_square(move);
	int destination_square = decode_destination_square(move);
	int piece_type = decode_piece_type(move);
	int promotedPieceType = decode_promoted_piece_type(move);

	std::cout << square_to_coordinates[start_square];
	std::cout << square_to_coordinates[destination_square];

	// print the promoted piece if available
	// promoted piece flag is active
	if (promotedPieceType)
	{
		if (promotedPieceType == Q || promotedPieceType == q)
		{
			// print piece
			std::cout << "q";
		}
		else if (promotedPieceType == R || promotedPieceType == r)
		{
			std::cout << "r";
		}
		else if (promotedPieceType == B || promotedPieceType == b)
		{
			std::cout << "b";
		}
		else if (promotedPieceType == N || promotedPieceType == n)
		{
			std::cout << "n";
		}
	}
}

void printMoveList(std::vector<unsigned long long> const& moves)
{
	for (int i = 0; i < moves.size(); i++)
	{
		printMove(moves[i]);
		std::cout << std::endl;
	}
	std::cout << "done" << std::endl;
}

