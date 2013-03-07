#include "GameManager.h"
#include <iostream>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <deque>

/**
 * Simple program that starts our game manager
 */
int main(int argc, char *argv[]) 
{
	std::shared_ptr<GameManager> game;

	game.reset(new GameManager());
	if(argv[1] != NULL)
		game->SetModelToLoad("models/room.obj");
	game->init();
	game->play();
	game.reset();
	return 0;
}
