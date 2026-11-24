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
			DEFAULT_MAX_HP = 250,
			DEFAULT_MAX_MP = 0,
			DEFAULT_ATK = 80,
			DEFAULT_DEF = 20,
			DEFAULT_MDEF = 20,
			DEFAULT_SPEED = 20;

	protected:
		void _Load() override;

	public:
		[[nodiscard]] std::optional<SkillInfo> SelectSkill(const SharedPtr<BattleCharacter>& target = nullptr) override;
		[[nodiscard]] std::vector<BoardOffset> GetRelativeMoveOffsets() const override;
	};
}
