#include "RareGolem.h"
#include <Math/Utils.h>

namespace Game {
	void RareGolem::_Load() {
		EnemyBattleCharacter::_Load();
		SetInfo({
			"レアゴーレム",
			"enemy/golem/icon.png",
			{
				DEFAULT_MAX_HP, 0, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
				DEFAULT_MAX_HP, 0, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
				{ RareGolemAnimation::Idle, "enemy/golem/idle.png" },
				{ RareGolemAnimation::Attack, "enemy/golem/idle.png" },
				{ RareGolemAnimation::Damage, "enemy/golem/idle.png" },
				{ RareGolemAnimation::Death, "enemy/golem/idle.png" }
			},
			CoreBackend::Drawing::Color(100, 160, 200)
		});
		SetStats({
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
		});
		SetSkills({
			{ "たいあたり", "physical", "normal", "chara/golem/attack/attack_1.lua" }
		});
		SetRustleParams(20.0, 4, 0.12);

		Initialize();

		// 上位種は元のテクスチャを赤系に染めて区別する
		SetModulateAll(Vector3D(255, 120, 120));
	}

	std::optional<SkillInfo> RareGolem::SelectSkill(const SharedPtr<BattleCharacter>& target) {
		const auto& skills = GetSkills();
		if (skills.empty()) {
			return std::nullopt;
		}
		const int skillCount = static_cast<int>(skills.size());
		const int index = GetRandomInt(0, skillCount);
		const int clampedIndex = Clamp(index, 0, skillCount - 1);
		return skills[clampedIndex];
	}

	std::vector<BoardOffset> RareGolem::GetRelativeMoveOffsets() const {
		return {
			{ 0, -1 },
			{ -1, 0 }, { 1, 0 },
			{ 0, 1 }
		};
	}
}
