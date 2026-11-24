#pragma once
#include "BattleCharacter.h"

namespace Game {
	namespace GouAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION,
			AttackPrepare,
			AttackPrepare2
		};
	}

	class Gou : public BattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 500,
			DEFAULT_MAX_MP = 200,
			DEFAULT_ATK = 500,
			DEFAULT_DEF = 60,
			DEFAULT_MDEF = 0,
			DEFAULT_SPEED = 10;
	protected:
		void _Load() override;
	};
}