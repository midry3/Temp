#include "RareSlime.h"
#include <Math/Utils.h>

namespace Game {
	void RareSlime::_Load() {
		EnemyBattleCharacter::_Load();
		SetInfo({
			"レアスライム",
			"enemy/rare_slime/icon.png",
			{
				DEFAULT_MAX_HP, 0, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
				DEFAULT_MAX_HP, 0, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
				{ RareSlimeAnimation::Idle, "enemy/rare_slime/idle.png" },
				{ RareSlimeAnimation::Attack, "enemy/rare_slime/idle.png" },
				{ RareSlimeAnimation::Damage, "enemy/rare_slime/idle.png" },
				{ RareSlimeAnimation::Death, "enemy/rare_slime/idle.png" }
			},
			CoreBackend::Drawing::Color(150, 200, 100)
		});
		SetStats({
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
		});
		SetSkills({
			{ "たいあたり", "physical", "normal", "chara/rare_slime/attack/attack_1.lua" }
		});
		SetRustleParams(20.0, 4, 0.12);

		Initialize();
	}

	std::optional<SkillInfo> RareSlime::SelectSkill(const SharedPtr<BattleCharacter>& target) {
		const auto& skills = GetSkills();
		if (skills.empty()) {
			return std::nullopt;
		}
		const int skillCount = static_cast<int>(skills.size());
		const int index = GetRandomInt(0, skillCount);
		const int clampedIndex = Clamp(index, 0, skillCount - 1);
		return skills[clampedIndex];
	}

	std::vector<BoardOffset> RareSlime::GetRelativeMoveOffsets() const {
		return {
			{ 0, -1 },
			{ -1, 0 }, { 1, 0 },
			{ 0, 1 }
		};
	}
}
