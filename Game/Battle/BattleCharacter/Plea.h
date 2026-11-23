#pragma once
#include "BattleCharacter.h"

namespace Game {
	namespace PleaAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION,
			Chanting
		};
	}

	class Plea : public BattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 500,
			DEFAULT_MAX_MP = 300,
			DEFAULT_ATK = 80,
			DEFAULT_DEF = 5,
			DEFAULT_MDEF = 30,
			DEFAULT_SPEED = 20;
	protected:
		void _Load() override;
	};
}