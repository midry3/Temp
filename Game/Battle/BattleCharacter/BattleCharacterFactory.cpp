#include "BattleCharacterFactory.h"
#include "Plea.h"
#include "Reko.h"
#include "Zon.h"
#include "Gou.h"
#include "Enemy/Slime.h"
#include "Enemy/Witch.h"
#include "Enemy/Golem.h"
#include "Enemy/RareSlime.h"
#include "Enemy/RareWitch.h"
#include "Enemy/RareGolem.h"

namespace Game {
	const char* BoardCharacterTypeKey(BoardCharacterType type) {
		switch (type) {
		case BoardCharacterType::Plea: return "plea";
		case BoardCharacterType::Reko: return "reko";
		case BoardCharacterType::Zon: return "zon";
		case BoardCharacterType::Gou: return "gou";
		case BoardCharacterType::Slime: return "slime";
		case BoardCharacterType::Witch: return "witch";
		case BoardCharacterType::Golem: return "golem";
		case BoardCharacterType::RareSlime: return "rare_slime";
		case BoardCharacterType::RareWitch: return "rare_witch";
		case BoardCharacterType::RareGolem: return "rare_golem";
		default: return "plea";
		}
	}

	BoardCharacterType ParseBoardCharacterType(const std::string& key) {
		if (key == "reko") return BoardCharacterType::Reko;
		if (key == "zon") return BoardCharacterType::Zon;
		if (key == "gou") return BoardCharacterType::Gou;
		if (key == "slime") return BoardCharacterType::Slime;
		if (key == "witch") return BoardCharacterType::Witch;
		if (key == "golem") return BoardCharacterType::Golem;
		if (key == "rare_slime") return BoardCharacterType::RareSlime;
		if (key == "rare_witch") return BoardCharacterType::RareWitch;
		if (key == "rare_golem") return BoardCharacterType::RareGolem;
		return BoardCharacterType::Plea;
	}

	SharedPtr<BattleCharacter> CreateBattleCharacter(BoardCharacterType type) {
		switch (type) {
		case BoardCharacterType::Plea: return MakeShared<Plea>();
		case BoardCharacterType::Reko: return MakeShared<Reko>();
		case BoardCharacterType::Zon: return MakeShared<Zon>();
		case BoardCharacterType::Gou: return MakeShared<Gou>();
		case BoardCharacterType::Slime: return MakeShared<Slime>();
		case BoardCharacterType::Witch: return MakeShared<Witch>();
		case BoardCharacterType::Golem: return MakeShared<Golem>();
		case BoardCharacterType::RareSlime: return MakeShared<RareSlime>();
		case BoardCharacterType::RareWitch: return MakeShared<RareWitch>();
		case BoardCharacterType::RareGolem: return MakeShared<RareGolem>();
		default: return nullptr;
		}
	}
}
