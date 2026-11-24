#pragma once
#include "EnemyBattleCharacter.h"

namespace Game {
	namespace WitchAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION
		};
	}

	class Witch : public EnemyBattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 120,
			DEFAULT_MAX_MP = 20,
			DEFAULT_ATK = 100,
			DEFAULT_DEF = 0,
			DEFAULT_MDEF = 50,
			DEFAULT_SPEED = 20;

	protected:
		void _Load() override;

	public:
		[[nodiscard]] std::optional<SkillInfo> SelectSkill(const SharedPtr<BattleCharacter>& target = nullptr) override;
		[[nodiscard]] std::vector<BoardOffset> GetRelativeMoveOffsets() const override;
	};
}
