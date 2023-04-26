#pragma once
#include "model.h"
#include "zobrist.h"

#include <chrono>

long perftTest(Model* startPosition, Zobrist* zTables, int depth, bool info);


void perftTestRecursive(Model* position, Zobrist* zTables, int depth, long& count, long& captures, long& enpassant, long& castles);

void perftEachMove(Model* startPosition, Zobrist* zTables, int depth);