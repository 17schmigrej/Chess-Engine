#include "environment.h"
#include "agent.h"
#include "helper.h"
#include "defs.h"
#include "attack_tables.h"
#include "perft.h"
#include "zobrist.h"
#include "uci.h"

#include <iostream>
#include <windows.h>
#include <tchar.h>

int main()
{

    // set the rng
    std::srand(std::time(nullptr));

    Zobrist zobristTables;
    Environment env(&zobristTables);
    Agent agent(&zobristTables);

    env.PrintBoard();
    std::cout << std::endl;

    UCILoop(&env, &agent);

    return 0;
}