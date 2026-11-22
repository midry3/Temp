#include "Slime.h"
#include <Math/Utils.h>

namespace Game {
	void Slime::_Load() {
		EnemyBattleCharacter::_Load();
		SetInfo({
			"スライム",
			{
				{ SlimeAnimation::Idle, "enemy/slime/idle.png" },
				{ SlimeAnimation::Attack, "enemy/slime/idle.png" },
				{ SlimeAnimation::Damage, "enemy/slime/idle.png" },
				{ SlimeAnimation::Death, "enemy/slime/idle.png" }
			}
		});
		SetStats({
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_SPEED
		});
		SetSkills({
			{ "たいあたり", "physical", "normal", "" },
			{ "ねばねばアタック", "physical", "nature", "" },
			{ "カサカサ突進", "special", "normal", "" }
		});
		SetRustleParams(20.0, 4, 0.12);

		Initialize();
	}

	std::optional<SkillInfo> Slime::SelectSkill(const SharedPtr<BattleCharacter>& target) {
		const auto& skills = GetSkills();
		if (skills.empty()) {
			return std::nullopt;
		}
		const int skillCount = static_cast<int>(skills.size());
		const int index = GetRandomInt(0, skillCount);
		const int clampedIndex = Clamp(index, 0, skillCount - 1);
		return skills[clampedIndex];
	}

	std::vector<BoardOffset> Slime::GetRelativeMoveOffsets() const {
		return {
			{ 0, -1 },
			{ -1, 0 }, { 1, 0 },
			{ 0, 1 }
		};
	}
}
