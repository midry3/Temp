#include <Novot.h>
#include "Game/Battle/TestBattle.h"

void Novot::Setup() {
	SceneManager.SetCurrentScene<Game::TestBattle>();
}

int Novot::Main() {
	PropertyManager.SetWindowSize({1280, 720});

	return Novot::StartGame();
}