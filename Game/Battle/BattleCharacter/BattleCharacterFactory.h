#pragma once
#include "BattleCharacter.h"
#include "../../Board/BoardTypes.h"

namespace Game {
	const char* BoardCharacterTypeKey(BoardCharacterType type);
	BoardCharacterType ParseBoardCharacterType(const std::string& key);
	SharedPtr<BattleCharacter> CreateBattleCharacter(BoardCharacterType type);
}
