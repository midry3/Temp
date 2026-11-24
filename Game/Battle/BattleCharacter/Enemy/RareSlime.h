#pragma once
#include "EnemyBattleCharacter.h"

namespace Game {
	namespace RareSlimeAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION
		};
	}

	// スライムの上位種。当面はスライムのコピー＋赤系の色味で区別する
	class RareSlime : public EnemyBattleCharacter {
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
