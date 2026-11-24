#pragma once
#include "EnemyBattleCharacter.h"

namespace Game {
	namespace RareGolemAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION
		};
	}

	// ゴーレムの上位種。当面はゴーレムのコピー＋赤系の色味で区別する
	class RareGolem : public EnemyBattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 2500,
			DEFAULT_MAX_MP = 0,
			DEFAULT_ATK = 500,
			DEFAULT_DEF = 500,
			DEFAULT_MDEF = 0,
			DEFAULT_SPEED = 5;

	protected:
		void _Load() override;

	public:
		[[nodiscard]] std::optional<SkillInfo> SelectSkill(const SharedPtr<BattleCharacter>& target = nullptr) override;
		[[nodiscard]] std::vector<BoardOffset> GetRelativeMoveOffsets() const override;
	};
}
