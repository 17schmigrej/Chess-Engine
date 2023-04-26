#pragma once
#include "defs.h"
#include "helper.h"
#include "attack_tables.h"
#include "zobrist.h"


#include <iostream>
#include <vector>
#include <string>
#include <ctype.h>
#include <chrono>



class Model {
public:
	Model(AttackTables* attackTable, Zobrist* zTables);
	Model(AttackTables* attackTable, Zobrist* zTables, int side, int enpassant, int castle, int halfmove, int fullmove,
		std::vector<unsigned long long>const& bitboards, std::vector<unsigned long long>& occ, unsigned long long hash);
	~Model();

	void SetBitboards(std::vector<unsigned long long> bbs);
	void SetOccupancies(std::vector<unsigned long long> occ);
	void SetSideToMove(int stm);
	void SetEnpassant(int ep);
	void SetCastlingRights(int castle);
	void SetHalfMoveClock(int hmc);
	void SetFullMoves(int fm);
	void SetHash(unsigned long long hash);

	std::vector<unsigned long long> GetBitboards() { return mBitboards; };
	std::vector<unsigned long long> GetOccupancies() {return mOccupancies; };
	unsigned long long GetBitboard(int piece) { return mBitboards[piece]; };
	int GetSideToMove() { return mSideToMove; };
	int GetEnpassant() { return mEnPassant; };
	int GetCastlingRights() { return mCastleRights; };
	int GetHalfMoveClock() { return mHalfMoveClock; };
	int GetFullMoves() { return mFullMoves; };
	unsigned long long GetHash() { return mHash; };

	void LoadPosition(std::string FEN, Zobrist *zTables);
	void PrintBoard();
	void PrintBitboard(int piece);
	void PrintAllBitboards();
	void PrintOccupancy(int side);
	void PrintAttackedSquares(int side);
	bool IsIllegal();
	bool KingCaptured();

	int Done(Zobrist *zTables);
	int CheckDrawOrCheckmateOrInPlay(Zobrist* zTables);

	long perftTest(int depth, bool info);

	void perftTestRecursive(int depth, long& count, long& captures, long& enpassant, long& castles);

	void perftEachMove(int depth);

	bool OnlyPawns();

	inline void generateMoves(std::vector<unsigned long long> &movesList)
	{
		// define source and target squares
		int sourceSquare, targetSquare;

		// define current piece's bitboard copy
		unsigned long long bitboard, attacks;

		// loop over all of the bitboards
		for (int piece = P; piece <= k; piece++)
		{
			// initialize piece bitboard copy so we can pop bits off of it
			bitboard = mBitboards[piece];

			// generate white pawns and white king castling moves
			if (mSideToMove == white)
			{
				// pick up white pawn bitboard's index
				if (piece == P)
				{
					// loop over the white pawn bitboard copy
					while (bitboard)
					{
						// initialize source square 
						sourceSquare = get_LS1B_index(bitboard);
						
						// initialize target square
						targetSquare = sourceSquare + 8;

						// generate quiet moves
						// if targe is on the board and the target is not occupied
						if (!(targetSquare > h8) && !get_bit(mOccupancies[both], targetSquare))
						{
							// pawn promotion
							if (sourceSquare >= a7 && sourceSquare <= h7)
							{
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: q " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: r " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: b " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: n " << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, Q, 0, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, R, 0, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, B, 0, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, N, 0, 0, 0, 0));
							}
							else
							{
								// one square ahead pawn move
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn push: " << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
								
								// two squares ahead pawn move
								if ((sourceSquare >= a2 && sourceSquare <= h2) && !get_bit(mOccupancies[both], targetSquare + 8))
								{
									//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare + 8] << "\tdouble pawn push: " << std::endl;
									movesList.push_back(encode_move(sourceSquare, targetSquare + 8, piece, 0, 0, 1, 0, 0));
								}
							}
							
						}
						// initialize pawn attacks bitboard
						attacks = mAttackTables->mPawnAttacks[mSideToMove][sourceSquare] & mOccupancies[black];

						// generate pawn captures
						while (attacks)
						{
							// initialize target square
							targetSquare = get_LS1B_index(attacks);

							// pawn promotion
							if (sourceSquare >= a7 && sourceSquare <= h7)
							{
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: q " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: r " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: b " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: n " << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, Q, 1, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, R, 1, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, B, 1, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, N, 1, 0, 0, 0));
							}
							else
							{
								// one square ahead diagonally pawn capture move
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn capture: " << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
							}

							// pop lsb from attack bitboard
							clear_bit(attacks, targetSquare);
						}

						// generate enpassant captures
						if (mEnPassant != noSquare)
						{
							// lookup pawn attacks and bitwise AND with enpassant square
							unsigned long long enpassantAttacks = mAttackTables->mPawnAttacks[mSideToMove][sourceSquare] & ((unsigned long long)1 << mEnPassant);

							// make sure enpassant capture is available
							if (enpassantAttacks)
							{
								// initialize enpassant capture target square
								int targetEnpassant = get_LS1B_index(enpassantAttacks);
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetEnpassant] << "\tpawn enpassant capture: " << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetEnpassant, piece, 0, 1, 0, 1, 0));

							}
						}

						// pop lsb from the copy
						clear_bit(bitboard, sourceSquare);
					}
				}

				// castling moves
				if (piece == K)
				{
					// king side castling is available
					if (mCastleRights & wk)
					{
						// make sure squares between king and kingside rook are empty
						if (!get_bit(mOccupancies[both], f1) && !get_bit(mOccupancies[both], g1))
						{
							// make sure king square and the f1 square are not under attack
							if (!mAttackTables->SquareAttacked(e1, black, mBitboards, mOccupancies) && !mAttackTables->SquareAttacked(f1, black, mBitboards, mOccupancies))
							{
								//std::cout << "e1g1\tcastling move" << std::endl;
								movesList.push_back(encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
							}
						}
					}

					// queen side castling is available
					if (mCastleRights & wq)
					{
						// make sure squares between king and queenside rook are empty
						if (!get_bit(mOccupancies[both], d1) && !get_bit(mOccupancies[both], c1) && !get_bit(mOccupancies[both], b1))
						{
							// make sure king square and the d1 square are not under attack
							if (!mAttackTables->SquareAttacked(e1, black, mBitboards, mOccupancies) && !mAttackTables->SquareAttacked(d1, black, mBitboards, mOccupancies))
							{
								//std::cout << "e1c1\tcastling move" << std::endl;
								movesList.push_back(encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
							}
						}

					}
				}
			}

			// generate black pawns and black king castling moves
			else
			{
				// pick up black pawn bitboard's index
				if (piece == p)
				{
					// loop over the black pawn bitboard copy
					while (bitboard)
					{
						// initialize source square 
						sourceSquare = get_LS1B_index(bitboard);

						// initialize target square
						targetSquare = sourceSquare - 8;

						// generate quiet moves
						// if targe is on the board and the target is not occupied
						if (!(targetSquare < 0) && !get_bit(mOccupancies[both], targetSquare))
						{
							// pawn promotion
							if (sourceSquare >= a2 && sourceSquare <= h2)
							{
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: q " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: r " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: b " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: n " << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, q, 0, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, r, 0, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, b, 0, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, n, 0, 0, 0, 0));

							}
							else
							{
								// one square ahead pawn move
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn push: " << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));

								// two squares ahead pawn move
								if ((sourceSquare >= a7 && sourceSquare <= h7) && !get_bit(mOccupancies[both], targetSquare - 8))
								{
									//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare - 8] << "\tdouble pawn push: " << std::endl;
									movesList.push_back(encode_move(sourceSquare, targetSquare - 8, piece, 0, 0, 1, 0, 0));
								}
							}

						}

						// initialize pawn attacks bitboard
						attacks = mAttackTables->mPawnAttacks[mSideToMove][sourceSquare] & mOccupancies[white];

						// generate pawn captures
						while (attacks)
						{
							// initialize target square
							targetSquare = get_LS1B_index(attacks);

							// pawn capture promotions
							if (sourceSquare >= a2 && sourceSquare <= h2)
							{
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: q " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: r " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: b " << std::endl;
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn promotion capture: n " << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, q, 1, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, r, 1, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, b, 1, 0, 0, 0));
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, n, 1, 0, 0, 0));

							}
							else
							{
								// one square ahead pawn move
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tpawn capture:" << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
							}

							// pop lsb from attack bitboard
							clear_bit(attacks, targetSquare);
						}

						// generate enpassant captures
						if (mEnPassant != noSquare)
						{
							// lookup pawn attacks and bitwise AND with enpassant square
							unsigned long long enpassantAttacks = mAttackTables->mPawnAttacks[mSideToMove][sourceSquare] & ((unsigned long long)1 << mEnPassant);

							// make sure enpassant capture is available
							if (enpassantAttacks)
							{
								// initialize enpassant capture target square
								int targetEnpassant = get_LS1B_index(enpassantAttacks);
								//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetEnpassant] << "\tpawn enpassant capture" << std::endl;
								movesList.push_back(encode_move(sourceSquare, targetEnpassant, piece, 0, 1, 0, 1, 0));

							}
						}
						
						// pop lsb from the copy
						clear_bit(bitboard, sourceSquare);
					}
				}

				// castling moves
				if (piece == k)
				{
					// king side castling is available
					if (mCastleRights & bk)
					{
						// make sure squares between king and kingside rook are empty
						if (!get_bit(mOccupancies[both], f8) && !get_bit(mOccupancies[both], g8))
						{
							// make sure king square and the f8 square are not under attack
							if (!mAttackTables->SquareAttacked(e8, white, mBitboards, mOccupancies) && !mAttackTables->SquareAttacked(f8, white, mBitboards, mOccupancies))
							{
								//std::cout << "e8g8\tcastling move" << std::endl;
								movesList.push_back(encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
							}
						}
					}

					// queen side castling is available
					if (mCastleRights & bq)
					{
						// make sure squares between king and queenside rook are empty
						if (!get_bit(mOccupancies[both], d8) && !get_bit(mOccupancies[both], c8) && !get_bit(mOccupancies[both], b8))
						{
							// make sure king square and the d8 square are not under attack
							if (!mAttackTables->SquareAttacked(e8, white, mBitboards, mOccupancies) && !mAttackTables->SquareAttacked(d8, white, mBitboards, mOccupancies))
							{
								//std::cout << "e8c8\tcastling move" << std::endl;
								movesList.push_back(encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
							}
						}

					}
				}
			}

			// generate knight moves
			if ((mSideToMove == white) ? piece == N : piece == n)
			{
				// loop over source squares of piece bitboard copy
				while (bitboard)
				{
					// init source square
					sourceSquare = get_LS1B_index(bitboard);

					// initialize piece attacks in order to get set of target squares (don't allow same side captures)
					attacks = mAttackTables->mKnightAttacks[sourceSquare] & ((mSideToMove == white) ? ~mOccupancies[white] : ~mOccupancies[black]);

					// loop over target squares available from genereated attacks
					while (attacks)
					{
						// initialize target square
						targetSquare = get_LS1B_index(attacks);

						// quiet moves
						if (!get_bit(((mSideToMove == white) ? mOccupancies[black] : mOccupancies[white]), targetSquare)) {

							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tknight quiet move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
						}
						else
						{
							// captures
							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tknight capture move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
						}
						// pop 
						clear_bit(attacks, targetSquare);
					}

					// pop bit from bitboard
					clear_bit(bitboard, sourceSquare);
				}
			}

			// genereate bishop moves
			if ((mSideToMove == white) ? piece == B : piece == b)
			{
				// loop over source squares of piece bitboard copy
				while (bitboard)
				{
					// init source square
					sourceSquare = get_LS1B_index(bitboard);

					// initialize piece attacks in order to get set of target squares (don't allow same side captures)
					attacks = mAttackTables->getBishopAttacks(sourceSquare, mOccupancies[both]) & ((mSideToMove == white) ? ~mOccupancies[white] : ~mOccupancies[black]);

					// loop over target squares available from genereated attacks
					while (attacks)
					{
						// initialize target square
						targetSquare = get_LS1B_index(attacks);

						// quiet moves
						if (!get_bit(((mSideToMove == white) ? mOccupancies[black] : mOccupancies[white]), targetSquare)) {

							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tbishop quiet move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
						}
						else
						{
							// captures
							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tbishop capture move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
						}
						// pop 
						clear_bit(attacks, targetSquare);
					}

					// pop bit from bitboard
					clear_bit(bitboard, sourceSquare);
				}
			}

			// generate rook moves
			if ((mSideToMove == white) ? piece == R : piece == r)
			{
				// loop over source squares of piece bitboard copy
				while (bitboard)
				{
					// init source square
					sourceSquare = get_LS1B_index(bitboard);

					// initialize piece attacks in order to get set of target squares (don't allow same side captures)
					attacks = mAttackTables->getRookAttacks(sourceSquare, mOccupancies[both]) & ((mSideToMove == white) ? ~mOccupancies[white] : ~mOccupancies[black]);

					// loop over target squares available from genereated attacks
					while (attacks)
					{
						// initialize target square
						targetSquare = get_LS1B_index(attacks);

						// quiet moves
						if (!get_bit(((mSideToMove == white) ? mOccupancies[black] : mOccupancies[white]), targetSquare)) {

							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\trook quiet move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
						}
						else
						{
							// captures
							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\trook capture move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
						}
						// pop 
						clear_bit(attacks, targetSquare);
					}

					// pop bit from bitboard
					clear_bit(bitboard, sourceSquare);
				}
			}

			// generate queen moves
			if ((mSideToMove == white) ? piece == Q : piece == q)
			{
				// loop over source squares of piece bitboard copy
				while (bitboard)
				{
					// init source square
					sourceSquare = get_LS1B_index(bitboard);

					// initialize piece attacks in order to get set of target squares (don't allow same side captures)
					attacks = mAttackTables->getQueenAttacks(sourceSquare, mOccupancies[both]) & ((mSideToMove == white) ? ~mOccupancies[white] : ~mOccupancies[black]);

					// loop over target squares available from genereated attacks
					while (attacks)
					{
						// initialize target square
						targetSquare = get_LS1B_index(attacks);

						// quiet moves
						if (!get_bit(((mSideToMove == white) ? mOccupancies[black] : mOccupancies[white]), targetSquare)) {

							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tqueen quiet move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
						}
						else
						{
							// captures
							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tqueen capture move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
						}
						// pop 
						clear_bit(attacks, targetSquare);
					}

					// pop bit from bitboard
					clear_bit(bitboard, sourceSquare);
				}
			}

			// generate king moves
			if ((mSideToMove == white) ? piece == K : piece == k)
			{
				// loop over source squares of piece bitboard copy
				while (bitboard)
				{
					// init source square
					sourceSquare = get_LS1B_index(bitboard);

					// initialize piece attacks in order to get set of target squares (don't allow same side captures)
					attacks = mAttackTables->mKingAttacks[sourceSquare] & ((mSideToMove == white) ? ~mOccupancies[white] : ~mOccupancies[black]);

					// loop over target squares available from genereated attacks
					while (attacks)
					{
						// initialize target square
						targetSquare = get_LS1B_index(attacks);

						// quiet moves
						if (!get_bit(((mSideToMove == white) ? mOccupancies[black] : mOccupancies[white]), targetSquare)) {

							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tking quiet move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
						}
						else
						{
							// captures
							//std::cout << square_to_coordinates[sourceSquare] << square_to_coordinates[targetSquare] << "\tking capture move" << std::endl;
							movesList.push_back(encode_move(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
						}
						// pop 
						clear_bit(attacks, targetSquare);
					}

					// pop bit from bitboard
					clear_bit(bitboard, sourceSquare);
				}
			}
		}
	}



	void inline UpdateOccupancies()
	{
		mOccupancies[white] = mBitboards[P] | mBitboards[R] | mBitboards[N] | mBitboards[B] | mBitboards[Q] | mBitboards[K];
		mOccupancies[black] = mBitboards[p] | mBitboards[r] | mBitboards[n] | mBitboards[b] | mBitboards[q] | mBitboards[k];
		mOccupancies[both] = mOccupancies[white] | mOccupancies[black];
	}

	void inline MakeMove(unsigned long long move, int moveFlag)
	{
		// quiet moves
		if (moveFlag == allMoves)
		{
			// make a copy of the game state to create a move
			//Model modelCopy(mAttackTables, mSideToMove, mEnPassant, mCastleRights, mHalfMoveClock, mFullMoves, mBitboards, mOccupancies, mHash);

			// capture the old datamembers that aren't tracked by the moove encoding
			mOldCastleRights.push_back(mCastleRights);
			mOldEnpassant.push_back(mEnPassant);
			mOldHalfMoveClock.push_back(mHalfMoveClock);
			mOldHash.push_back(mHash);


			// extract the move information
			int start_square = decode_start_square(move);
			int destination_square = decode_destination_square(move);
			int piece_type = decode_piece_type(move);
			int promoted_piece_type = decode_promoted_piece_type(move);

			bool capture_flag = decode_capture_flag(move);
			bool enpassant_flag = decode_enpassant_flag(move);
			bool double_pawn_push_flag = decode_double_push_flag(move);
			bool castling_flag = decode_castling_flag(move);

			// move the piece
			clear_bit(mBitboards[piece_type], start_square);
			set_bit(mBitboards[piece_type], destination_square);

			// hash piece (remove piece from the start square and put it on the the destination square)
			mHash ^= mZobristTables->mRandomNumberPieceTable[piece_type][start_square];
			mHash ^= mZobristTables->mRandomNumberPieceTable[piece_type][destination_square];
			

			// now we need to clear the captured pieces bit if there was a cpature
			if (capture_flag)
			{
				// reset the 50 move rule
				SetHalfMoveClock(0);
				// we need to determine which side white/black we need to pop the bit from the bitboard
				int startPiece, endPiece;

				if (mSideToMove == white)
				{
					startPiece = p;
					endPiece = k;
				}
				else
				{
					startPiece = P;
					endPiece = K;
				}
				
				// now loop over the bitboards and clear the bit that had the captured piece
				for (int i = startPiece; i <= endPiece; i++)
				{
					if (get_bit(mBitboards[i], destination_square))
					{
						clear_bit(mBitboards[i], destination_square);

						// store the piece type
						mLastPieceCaptured.push_back(i);

						// hash the cleared piece 
						mHash ^= mZobristTables->mRandomNumberPieceTable[i][destination_square];
						break;
					}
				}
			}
			
			// handling promotion if necessary
			if (promoted_piece_type)
			{
				// remove the pawn from it's square
				clear_bit(mBitboards[piece_type], destination_square);

				// create a new piece
				set_bit(mBitboards[promoted_piece_type], destination_square);

				// unhash the pawn
				mHash ^= mZobristTables->mRandomNumberPieceTable[piece_type][destination_square];

				// hash the promoted piece
				mHash ^= mZobristTables->mRandomNumberPieceTable[promoted_piece_type][destination_square];
			}

			// handling enpassant moves
			if (enpassant_flag)
			{
				// remove the captured pawn depending on white's or black's turn
				if (mSideToMove == white)
				{
					clear_bit(mBitboards[p], destination_square - 8);
					mLastPieceCaptured.push_back(p);

					// hash the captured piece
					mHash ^= mZobristTables->mRandomNumberPieceTable[p][destination_square - 8];
				}
				else
				{
					clear_bit(mBitboards[P], destination_square + 8);

					// hash the captured piece
					mHash ^= mZobristTables->mRandomNumberPieceTable[P][destination_square + 8];
					mLastPieceCaptured.push_back(P);
				}


			}
			// undo the enpassant hash
			if (mEnPassant != noSquare)
			{
				mHash ^= mZobristTables->mRandomNumberEnpassantTable[mEnPassant];
			}
			
			// reset the enpassant square
			mEnPassant = noSquare;




			// handling double pawn pushes
			if (double_pawn_push_flag)
			{
				// set the en passant square depending on side to move
				if (mSideToMove == white)
				{
					// set the enpassant square
					mEnPassant = destination_square - 8;
					// hash the en passantsquare
					mHash ^= mZobristTables->mRandomNumberEnpassantTable[destination_square - 8];
				}
				else
				{
					// set the enpassant square
					mEnPassant = destination_square + 8;
					// hash the enpassant square
					mHash ^= mZobristTables->mRandomNumberEnpassantTable[destination_square + 8];
				}
			}

			// handling castling moves
			if (castling_flag)
			{
				// depending on the destination square we handle the castling options
				switch (destination_square)
				{
					// white kingside castle
					case (g1):
						// move H rook
						clear_bit(mBitboards[R], h1);
						set_bit(mBitboards[R], f1);

						// hash rook move
						mHash ^= mZobristTables->mRandomNumberPieceTable[R][h1];
						mHash ^= mZobristTables->mRandomNumberPieceTable[R][f1];
						break;

					// white queenside castle
					case (c1):
						// move A rook
						clear_bit(mBitboards[R], a1);
						set_bit(mBitboards[R], d1);

						// hash rook move
						mHash ^= mZobristTables->mRandomNumberPieceTable[R][a1];
						mHash ^= mZobristTables->mRandomNumberPieceTable[R][d1];
						break;

					// black kingside castle
					case (g8):
						clear_bit(mBitboards[r], h8);
						set_bit(mBitboards[r], f8);

						// hash rook move
						mHash ^= mZobristTables->mRandomNumberPieceTable[r][h8];
						mHash ^= mZobristTables->mRandomNumberPieceTable[r][f8];
						break;

					// black queensinde castle
					case (c8):
						clear_bit(mBitboards[r], a8);
						set_bit(mBitboards[r], d8);

						// hash rook move
						mHash ^= mZobristTables->mRandomNumberPieceTable[r][a8];
						mHash ^= mZobristTables->mRandomNumberPieceTable[r][d8];
						break;
				}
			}

			// unhash castling rights
			mHash ^= mZobristTables->mRandomNumberCastleTable[mCastleRights];

			// update the castling rights
			// handles when a king or rook moves
			mCastleRights &= castlingRightsConstants[start_square];
			// this is needed for the situations of when a piece captures a rook
			mCastleRights &= castlingRightsConstants[destination_square];

			// hash castling rights
			mHash ^= mZobristTables->mRandomNumberCastleTable[mCastleRights];


			// update the occupancies
			UpdateOccupancies();

			// change side-to-move
			mSideToMove ^= 1;

			// hash side
			mHash ^= mZobristTables->mRandomNumberSide;

			// check if the previous move left the king in check
			if (mAttackTables->SquareAttacked((mSideToMove == white) ?
													get_LS1B_index(mBitboards[k]) :
													get_LS1B_index(mBitboards[K]),
												mSideToMove,
												mBitboards,
												mOccupancies))
			{
				// move is illegal, update
				mIllegalState = 1;
			}

			// update the half-move clock
			if (piece_type == P || piece_type == p)
			{
				mHalfMoveClock = 0;
			}
			else
			{
				mHalfMoveClock += 1;
			}

			// update the full-move clock
			if (mSideToMove == white)
			{
				mFullMoves += 1;
			}

		}

		// capture moves
		else
		{
			// make sure move is actually a capture
			if (decode_capture_flag(move))
			{
				return MakeMove(move, moveFlag);
			}
			else
			{
				mIllegalState = 1;
			}
		}
	}

	void inline UnmakeMove(unsigned long long move)
	{
		// revert to legal
		mIllegalState = 0;

		// undo the full moves
		if (mSideToMove == white)
		{
			mFullMoves--;
		}

		// reverse the side to move
		mSideToMove ^= 1;

		// extract the move information
		int start_square = decode_start_square(move);
		int destination_square = decode_destination_square(move);
		int piece_type = decode_piece_type(move);
		int promoted_piece_type = decode_promoted_piece_type(move);

		bool capture_flag = decode_capture_flag(move);
		bool enpassant_flag = decode_enpassant_flag(move);
		bool double_pawn_push_flag = decode_double_push_flag(move);
		bool castling_flag = decode_castling_flag(move);

		// put the moved piece back to the starting square
		clear_bit(mBitboards[piece_type], destination_square);
		set_bit(mBitboards[piece_type], start_square);

		// if there was a capture, put the old piece back on
		if (capture_flag)
		{
			//std::cout << "Size of lastPieceCaptured: " << mLastPieceCaptured.size() << std::endl;
			// get the bitboard index of the last capture
			int index = mLastPieceCaptured[mLastPieceCaptured.size() - 1];
			mLastPieceCaptured.pop_back();

			// put the captured piece back onto it's square
			//std::cout << "Move: ";
			//printMove(move);
			//std::cout <<"index: " << index << std::endl;
			if (!enpassant_flag)
			{
				set_bit(mBitboards[index], destination_square);
			}
		}

		if (promoted_piece_type)
		{
			// remove the promoted piece
			clear_bit(mBitboards[promoted_piece_type], destination_square);

			// put the pawn back to it's starting square
			set_bit(mBitboards[piece_type], start_square);
		}

		// handling enpassant moves
		if (enpassant_flag)
		{
			// put back the captured pawn depending on white's or black's turn
			if (mSideToMove == white)
			{
				set_bit(mBitboards[p], destination_square - 8);
			}
			else
			{
				set_bit(mBitboards[P], destination_square + 8);
			}
		}

		// handling castling moves
		if (castling_flag)
		{
			// depending on the destination square we handle the castling options
			switch (destination_square)
			{
				// white kingside castle
			case (g1):
				// move H rook
				set_bit(mBitboards[R], h1);
				clear_bit(mBitboards[R], f1);
				break;

				// white queenside castle
			case (c1):
				// move A rook
				set_bit(mBitboards[R], a1);
				clear_bit(mBitboards[R], d1);
				break;

				// black kingside castle
			case (g8):
				set_bit(mBitboards[r], h8);
				clear_bit(mBitboards[r], f8);
				break;

				// black queensinde castle
			case (c8):
				set_bit(mBitboards[r], a8);
				clear_bit(mBitboards[r], d8);
				break;
			}
		}
	
		// revert castling rights, enpassant, halfmoves, and hash
		mEnPassant = mOldEnpassant[mOldEnpassant.size() - 1];
		mCastleRights = mOldCastleRights[mOldCastleRights.size() - 1];
		mHalfMoveClock = mOldHalfMoveClock[mOldHalfMoveClock.size() - 1];
		mHash = mOldHash[mOldHash.size() - 1];

		mOldEnpassant.pop_back();
		mOldCastleRights.pop_back();
		mOldHalfMoveClock.pop_back();
		mOldHash.pop_back();

		UpdateOccupancies();

	}

	int mIllegalState;
	int mSideToMove;
	int mEnPassant;
	int mCastleRights;
	int mHalfMoveClock;
	int mFullMoves;
	unsigned long long mHash;
	std::vector<unsigned long long> mBitboards;
	std::vector<unsigned long long> mOccupancies;

private:
	AttackTables* mAttackTables;
	Zobrist* mZobristTables;

	std::vector<int> mOldEnpassant;
	std::vector<int> mOldCastleRights;
	std::vector<int> mOldHalfMoveClock;
	std::vector<unsigned long long> mOldHash;
	std::vector<int> mLastPieceCaptured;
};
