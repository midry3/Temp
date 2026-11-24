#include "RareWitch.h"
#include <Math/Utils.h>

namespace Game {
	void RareWitch::_Load() {
		EnemyBattleCharacter::_Load();
		SetInfo({
			"上位ウィッチ",
			"enemy/rare_witch/icon.png",
			{
				DEFAULT_MAX_HP, 0, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
				DEFAULT_MAX_HP, 0, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
				{ RareWitchAnimation::Idle, "enemy/rare_witch/idle.png" },
				{ RareWitchAnimation::Attack, "enemy/rare_witch/idle.png" },
				{ RareWitchAnimation::Damage, "enemy/rare_witch/idle.png" },
				{ RareWitchAnimation::Death, "enemy/rare_witch/idle.png" }
			},
			CoreBackend::Drawing::Color(100, 160, 200)
			});
		SetStats({
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			});
		SetSkills({
			{ "魔法", "special", "normal", "chara/witch/attack/attack_1.lua" }
			});
		SetRustleParams(20.0, 4, 0.12);

		Initialize();
	}

	std::optional<SkillInfo> RareWitch::SelectSkill(const SharedPtr<BattleCharacter>& target) {
		const auto& skills = GetSkills();
		if (skills.empty()) {
			return std::nullopt;
		}
		const int skillCount = static_cast<int>(skills.size());
		const int index = GetRandomInt(0, skillCount);
		const int clampedIndex = Clamp(index, 0, skillCount - 1);
		return skills[clampedIndex];
	}

	std::vector<BoardOffset> RareWitch::GetRelativeMoveOffsets() const {
		return {
			{ 0, -1 },
			{ -1, 0 }, { 1, 0 },
			{ 0, 1 }
		};
	}
}
