#pragma once
#include "BattleCharacter.h"
#include "RekoAlpha.h"

namespace Game {
	namespace RekoAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION,
			AttackBeam,
			OpenPC,
			UsingPC,
			SwordPrepare,
			SwordAttack
		};
	}

	class Reko : public BattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 500,
			DEFAULT_MAX_MP = 250,
			DEFAULT_ATK = 100,
			DEFAULT_DEF = 80,
			DEFAULT_MDEF = 0,
			DEFAULT_SPEED = 10;
		SharedPtr<RekoAlpha> _alphaAnim;
	protected:
		void _Load() override;
	};
}