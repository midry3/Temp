#include "CommonData.h"

namespace Game {
	SharedPtr<BoardBase> CommonData::CurrentBoard = nullptr;
	bool CommonData::LastBattleEnemyDefeated = false;
}