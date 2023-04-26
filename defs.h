#pragma once

#include <iostream>
#include <vector>
#include <map>

struct Percepts
{
	std::vector<unsigned long long> bitboards;
	std::vector<unsigned long long> occupancies;
	int sideToMove;
	int enPassant;
	int castlingRights;
	int halfMoveClock;
	int fullMoves;
	unsigned long long hash;
	std::map<unsigned long long, int> movesHistory;
};

//bit macros
#define get_bit(bitboard, square) ((bitboard) & ((unsigned long long)1 << (square)))
#define set_bit(bitboard, square) ((bitboard) |= ((unsigned long long)1 << (square)))
#define clear_bit(bitboard, square) ((bitboard) &= ~((unsigned long long)1 << (square)))

const unsigned long long A_FILE = 0x0101010101010101;
const unsigned long long B_FILE = 0x202020202020202;
const unsigned long long C_FILE = 0x404040404040404;
const unsigned long long D_FILE = 0x808080808080808;
const unsigned long long E_FILE = 0x1010101010101010;
const unsigned long long F_FILE = 0x2020202020202020;
const unsigned long long G_FILE = 0x4040404040404040;
const unsigned long long H_FILE = 0x8080808080808080;

const unsigned long long RANK1 = 0x00000000000000FF;
const unsigned long long RANK2 = RANK1 << 8;
const unsigned long long RANK3 = RANK2 << 8;
const unsigned long long RANK4 = RANK3 << 8;
const unsigned long long RANK5 = RANK4 << 8;
const unsigned long long RANK6 = RANK5 << 8;
const unsigned long long RANK7 = RANK6 << 8;
const unsigned long long RANK8 = 0xFF00000000000000;


const unsigned long long AB_FILES = 0x303030303030303;
const unsigned long long GH_FILES = 0xc0c0c0c0c0c0c0c0;

const unsigned long long A1_H8_DIAGONAL = 0x8040201008040201;
const unsigned long long H1_A8_DIAGONAL = 0x0102040810204080;
const unsigned long long LIGHT_SQUARES = 0x55AA55AA55AA55AA;
const unsigned long long DARK_SQUARES = 0xAA55AA55AA55AA55;

const std::vector<unsigned long long> fileConstants = {
	A_FILE, B_FILE, C_FILE, D_FILE, E_FILE, F_FILE, G_FILE, H_FILE
};

const std::vector<unsigned long long> rankConstants = {
	RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8,
};
/*
				castling rights		castling       move			binary	dec
									rights	       update

	king and rooks didn't move:			1111   &   1111		=    1111	15

			  white king moved:			1111   &   1100		=	 1100	12
			white H rook moved:			1111   &   1110		=	 1110	14
			white A rook moved:			1111   &   1101		=	 1101	13

			  black king moved:			1111   &   0011		=	 0011	3
			black H rook moved:			1111   &   1011		=	 1011	11
			black A rook moved:			1111   &   0111		=	 0111	7
*/

// depending on the square that the piece moved bitwise AND the castling rights data members with these
const std::vector<int> castlingRightsConstants = {
	13, 15, 15, 15, 12, 15, 15, 14,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	 7, 15, 15, 15,  3, 15, 15, 11,
};


/*
				move encoding bits						    hex number mask

	0000 0000 0000 0000 0011 1111	starting square			0x34
	0000 0000 0000 1111 1100 0000	destination square		0xfc0
	0000 0000 1111 0000 0000 0000	piece					0xf000
	0000 1111 0000 0000 0000 0000	promoted piece			0xf0000
	0001 0000 0000 0000 0000 0000	capt. flag				0x100000
	0010 0000 0000 0000 0000 0000	double pawn push		0x200000
	0100 0000 0000 0000 0000 0000	enpassant capt. flag	0x400000
	1000 0000 0000 0000 0000 0000	castling flag			0x800000

*/

// encoding a move
#define encode_move(starting, destination, piece, promoted, capture, double_push, enpassant, castling) \
	(starting) |				\
	((destination) << 6) |		\
	((piece) << 12) |			\
	((promoted) << 16) |		\
	((capture) << 20) |			\
	((double_push) << 21) |		\
	((enpassant) << 22) |		\
	((castling) << 23)

// extracting the starting square
#define decode_start_square(move) ((move) & 0x3f)

// extracting the destination square
#define decode_destination_square(move) (((move) & 0xfc0) >> 6)

// extracting the piece type
#define decode_piece_type(move) (((move) & 0xf000) >> 12)

// extracting the promoted piece type
#define decode_promoted_piece_type(move) (((move) & 0xf0000) >> 16)

// extracting the capture flag
#define decode_capture_flag(move) (((move) & 0x100000))

// extracting the double pawn push flag
#define decode_double_push_flag(move) (((move) & 0x200000))

// extracting the enpassant flag
#define decode_enpassant_flag(move) (((move) & 0x400000))

// extracting the castling flag
#define decode_castling_flag(move) (((move) & 0x800000))

//enum board squres

enum {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8,
	noSquare
};

// occupancies
enum {
	white, black, both
};

// for magic bitboard purposes
enum {
	rook, bishop
};

// for quiessence search purposes
enum {
	allMoves, onlyCaptures
};

// castling rights (binary representation)
enum {
	wk = 1, wq = 2, bk = 4, bq = 8 
};

// pieces, upper = white, lower = black
enum {
	P, N, B, R, Q, K, 
	p, n, b, r, q, k, pieceLast
};


// end states
enum {
	draw, checkmate, inplay
};

const std::string asciiPieces = "PNBRQKpnbrqk";

const std::map<char, int> asciiToConstant = {
	{'P', P},
	{'N', N},
	{'B', B},
	{'R', R},
	{'Q', Q},
	{'K', K},
	{'p', p},
	{'n', n},
	{'b', b},
	{'r', r},
	{'q', q},
	{'k', k}
};

// this is used for UCI protocol
const std::map<int, char> promotedPieces = {
	{P, 'p'},
	{N, 'n'},
	{B, 'b'},
	{R, 'r'},
	{Q, 'q'},
	{K, 'k'},
	{p, 'p'},
	{n, 'n'},
	{b, 'b'},
	{r, 'r'},
	{q, 'q'},
	{k, 'k'}
};

const std::vector <std::string> square_to_coordinates = {
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};


