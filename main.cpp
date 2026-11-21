#include <Novot.h>
#include "Game/Board/TestBoard.h"
#include "Game/Battle/TestBattle.h"

void NovotSetup() {
	SceneManager.SetCurrentScene<Game::TestBoard>();
}

int NovotMain() {
	PropertyManager.SetWindowSize({1280, 720});
	PropertyManager.SetFPS(60);

	return Novot::StartGame();
}