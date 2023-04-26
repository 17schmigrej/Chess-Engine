#pragma once

#include <vector>

class Model;

class Zobrist
{
public:
	Zobrist();

	void InitializeRandomNumbers();
	unsigned long long HashBoard(Model* position);

	std::vector < std::vector<unsigned long long> > mRandomNumberPieceTable;
	std::vector <unsigned long long> mRandomNumberCastleTable;
	std::vector < unsigned long long > mRandomNumberEnpassantTable;
	unsigned long long mRandomNumberSide;


};

