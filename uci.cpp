#include "uci.h"

unsigned long long Book(Environment* env, Agent* agent)
{
	unsigned long long move;
	if (agent->mModel.mHash == 0x9f8c3d6078abf32e)
	{
		move = encode_move(d2, d4, P, 0, 0, 1, 0, 0);
	}
	else if (agent->mModel.mHash == 0xbed2b34f41b07b4f)
	{
		move = encode_move(e7, e5, P, 0, 0, 1, 0, 0);
	}
	else if (agent->mModel.mHash == 0x68da7c526fdf142)
	{
		move = encode_move(d7, d5, P, 0, 0, 1, 0, 0);
	}
	else if (agent->mModel.mHash == 0x7797168ace31f09d)
	{
		move = encode_move(e7, e5, P, 0, 0, 1, 0, 0);
	}
	else if (agent->mModel.mHash == 0x875979e4a8ba2aaa)
	{
		move = encode_move(c2, c4, P, 0, 0, 1, 0, 0);
	}
	else if (agent->mModel.mHash == 0xf96b7117c55237a0)
	{
		move = encode_move(e7, e6, P, 0, 0, 0, 0, 0);
	}
	else if (agent->mModel.mHash == 0xd62c6c3e90aee817)
	{
		move = encode_move(e7, e6, P, 0, 0, 0, 0, 0);
	}
	else if (agent->mModel.mHash == 0x9aeaa4fcbed9b7b9)
	{
		move = encode_move(c1, f4, B, 0, 0, 0, 0, 0);
	}
	else if (agent->mModel.mHash == 0xe8424c77f1ad8f9)
	{
		// wayward queen attack defense
		move = encode_move(b8, c6, N, 0, 0, 0, 0, 0);
	}
	else if (agent->mModel.mHash == 0x9f86906f8d1a6a1a)
	{
		// wayward queen attack defense pawn move
		move = encode_move(g7, g6, P, 0, 0, 0, 0, 0);
	}
	else if (agent->mModel.mHash == 0x816f9db0d80a3ddf)
	{
		// position startpos moves e2e4 e7e5 g1f3
		move = encode_move(b8, c6, n, 0, 0, 0, 0, 0);
	}
	else if (agent->mModel.mHash == 0x9d0123354b21c08)
	{
		// position startpos moves e2e4 e7e5 g1f3 b8c6
		move = encode_move(f1, b5, B, 0, 0, 0, 0, 0);
	}
	else if (agent->mModel.mHash == 0xebd7ed5c5bc1b62a)
	{
		// position startpos moves e2e4 e7e5
		move = encode_move(f1, c4, B, 0, 0, 0, 0, 0);
	}
	else if (agent->mModel.mHash == 0x58d00ba39bcd2f2a)
	{
		// position startpos moves d2d4 d7d5 g1f3
		move = encode_move(c8, f5, B, 0, 0, 0, 0, 0);
	}
	else if (agent->mModel.mFullMoves == 0)
	{
		move = encode_move(d7, d5, P, 0, 0, 1, 0, 0);
	}
	else
	{
		// no book
		move = 0;
	}
	return move;
}

void UCILoop(Environment *env, Agent* agent)
{
	
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	// define user / GUI input buffer
	char input[2000];

	// print identifying info
	std::cout << "id name WolfTacticsV1" << std::endl;
	std::cout << "id author Greg Schmidt" << std::endl;
	std::cout << "uciok" << std::endl;

	// main loop
	while (true)
	{
		// reset user/GUI input
		memset(input, 0, sizeof(input));

		// make sure output reaches the GUI
		fflush(stdout);

		// get user/GUI input
		if (!fgets(input, 2000, stdin))
		{
			continue;
		}

		// make sure input is available
		if (input[0] == '\n')
		{
			continue;
		}

		// parse UCI "isready" command
		if (strncmp(input, "isready", 7) == 0)
		{
			std::cout << "readyok" << std::endl;
			continue;
		}

		else if (strncmp(input, "position", 8) == 0)
		{
			env->ParsePosition(input);
			env->PrintBoard();

			Percepts p = env->GetPercepts();
			agent->UpdateFromPercepts(p);
			continue;
		}

		else if (strncmp(input, "ucinewgame", 10) == 0)
		{
			// TODO make reset to start position
			env->ParsePosition("position startpos");
			env->PrintBoard();
		}
		else if (strncmp(input, "option", 6) == 0)
		{
			std::cout << "uciok" << std::endl;
		}

		else if (strncmp(input, "go", 2) == 0)
		{
			// opening book
			unsigned long long move;
			move = Book(env, agent);
			if (!move)
			{
				int depth = env->ParseGoDepth(input);
				move = agent->SearchNegamax(depth);
			}

			std::cout << "bestmove ";
			printMove(move);
			std::cout << std::endl;
			
		}

		else if (strncmp(input, "quit", 4) == 0)
		{
			// break out
			break;
		}

		// parse UCE "uci" command
		else if (strncmp(input, "uci", 3) == 0)
		{
			// print identifying info
			std::cout << "id name WolfTacticsV1" << std::endl;
			std::cout << "id author Greg Schmidt" << std::endl;
			std::cout << "uciok" << std::endl;
		}
	}

	
}
