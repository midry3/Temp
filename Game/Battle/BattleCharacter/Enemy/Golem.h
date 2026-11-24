#pragma once
#include "EnemyBattleCharacter.h"

namespace Game {
	namespace GolemAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION
		};
	}

	class Golem : public EnemyBattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 300,
			DEFAULT_MAX_MP = 0,
			DEFAULT_ATK = 60,
			DEFAULT_DEF = 30,
			DEFAULT_MDEF = 0,
			DEFAULT_SPEED = 8;

	protected:
		void _Load() override;

	public:
		[[nodiscard]] std::optional<SkillInfo> SelectSkill(const SharedPtr<BattleCharacter>& target = nullptr) override;
		[[nodiscard]] std::vector<BoardOffset> GetRelativeMoveOffsets() const override;
	};
}
