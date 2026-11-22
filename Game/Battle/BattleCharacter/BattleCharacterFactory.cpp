#include "BattleCharacterFactory.h"
#include "Plea.h"
#include "Reko.h"
#include "Zon.h"
#include "Gou.h"

namespace Game {
	const char* BoardCharacterTypeKey(BoardCharacterType type) {
		switch (type) {
		case BoardCharacterType::Plea: return "plea";
		case BoardCharacterType::Reko: return "reko";
		case BoardCharacterType::Zon: return "zon";
		case BoardCharacterType::Gou: return "gou";
		default: return "plea";
		}
	}

	BoardCharacterType ParseBoardCharacterType(const std::string& key) {
		if (key == "reko") return BoardCharacterType::Reko;
		if (key == "zon") return BoardCharacterType::Zon;
		if (key == "gou") return BoardCharacterType::Gou;
		return BoardCharacterType::Plea;
	}

	SharedPtr<BattleCharacter> CreateBattleCharacter(BoardCharacterType type) {
		switch (type) {
		case BoardCharacterType::Plea: return MakeShared<Plea>();
		case BoardCharacterType::Reko: return MakeShared<Reko>();
		case BoardCharacterType::Zon: return MakeShared<Zon>();
		case BoardCharacterType::Gou: return MakeShared<Gou>();
		default: return nullptr;
		}
	}
}
