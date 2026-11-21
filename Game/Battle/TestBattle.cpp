#include "TestBattle.h"
#include "BattleCharacter/Plea.h"
#include "BattleCharacter/Reko.h"
#include "BattleCharacter/Zon.h"
#include "BattleCharacter/Gou.h"

namespace Game {
	void TestBattle::_Load() {
		BattleBase::_Load();

		_allies.push_back(MakeShared<Reko>());

		_enemies.push_back(MakeShared<Reko>());
		PutCharacters();
	}
}