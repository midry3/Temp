#pragma once
#include <string>

namespace Game {
	// 技の対象範囲（単体／全体）
	enum class SkillTargetScope {
		Single,
		All
	};

	// 技の対象陣営（敵／味方）。デフォルトは敵。
	enum class SkillTargetSide {
		Enemy,
		Ally,
		Self
	};

	struct SkillInfo {
		std::string name;
		std::string primaryAttribute;
		std::string secondaryAttribute;
		std::string directionFile;
		int mpCost = 10;
		SkillTargetScope targetScope = SkillTargetScope::Single;
		SkillTargetSide targetSide = SkillTargetSide::Enemy;
	};
}
