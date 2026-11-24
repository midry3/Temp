#pragma once
#include "EnemyBattleCharacter.h"

namespace Game {
	namespace RareWitchAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION
		};
	}

	// ウィッチの上位種。当面はウィッチのコピー＋赤系の色味で区別する
	class RareWitch : public EnemyBattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 200,
			DEFAULT_MAX_MP = 0,
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
