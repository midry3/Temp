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
			DEFAULT_MAX_HP = 150,
			DEFAULT_MAX_MP = 0,
			DEFAULT_ATK = 40,
			DEFAULT_DEF = 10,
			DEFAULT_MDEF = 10,
			DEFAULT_SPEED = 15;

	protected:
		void _Load() override;

	public:
		[[nodiscard]] std::optional<SkillInfo> SelectSkill(const SharedPtr<BattleCharacter>& target = nullptr) override;
		[[nodiscard]] std::vector<BoardOffset> GetRelativeMoveOffsets() const override;
	};
}
