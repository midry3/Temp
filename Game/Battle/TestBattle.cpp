#include "TestBattle.h"
#include "BattleCharacter/Plea.h"
#include "BattleCharacter/Reko.h"

namespace Game {
	void TestBattle::_Load() {
		BattleBase::_Load();

		_allies.push_back(MakeShared<Plea>());

		_enemies.push_back(MakeShared<Reko>());
		PutCharacters();
	}
}