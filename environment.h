#pragma once
#include "helper.h"
#include "model.h"
#include "defs.h"
#include "zobrist.h"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>



class Environment {
public:
	Environment(Zobrist *zTables);
	~Environment();
	void PrintBoard();
	void PrintBitboard(int piece);
	void PrintOccupancy(int side);
	void LoadPosition(std::string FEN);
	Percepts GetPercepts();
	unsigned long long ParseMove(const char* moveString);
	void MakeMove(unsigned long long move);
	void ParsePosition(const char* command);
	int ParseGoDepth(const char* command);
	bool Done();
	void PrintMadeMoves();
	void ListCurrentPositionMoves();
	void UndoMove();
	
	Model mModel;
	AttackTables mAttackTables;
	Zobrist *mZobristTables;
	
	std::vector<unsigned long long> mMovesMade;
	std::vector<unsigned long long> mHashRepititionList;
	std::unordered_map<unsigned long long, int> mMoveHashesHistory;
	std::vector< std::vector<unsigned long long> > mZorbristRandomNumbers;


private:

};

