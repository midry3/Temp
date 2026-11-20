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
			DEFAULT_MAX_MP = 250,
			DEFAULT_ATK = 10,
			DEFAULT_DEF = 5,
			DEFAULT_SPEED = 10;
	protected:
		void _Load() override;
	};
}