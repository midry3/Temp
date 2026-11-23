#pragma once
#include "BattleCharacter.h"

namespace Game {
	namespace ZonAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION,
			GUN_SHOT_PREPARE,
			GUN_SHOT,
			Pick
		};
	}

	class Zon : public BattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 500,
			DEFAULT_MAX_MP = 250,
			DEFAULT_ATK = 80,
			DEFAULT_DEF = 20,
			DEFAULT_MDEF = 0,
			DEFAULT_SPEED = 10;
	protected:
		void _Load() override;
	};
}