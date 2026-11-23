#pragma once
#include "EnemyBattleCharacter.h"

namespace Game {
	namespace SlimeAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION
		};
	}

	class Slime : public EnemyBattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 80,
			DEFAULT_MAX_MP = 20,
			DEFAULT_ATK = 8,
			DEFAULT_DEF = 3,
			DEFAULT_MDEF = 0,
			DEFAULT_SPEED = 5;

	protected:
		void _Load() override;

	public:
		[[nodiscard]] std::optional<SkillInfo> SelectSkill(const SharedPtr<BattleCharacter>& target = nullptr) override;
		[[nodiscard]] std::vector<BoardOffset> GetRelativeMoveOffsets() const override;
	};
}
