#pragma once
#include "BattleCharacter.h"

namespace Game {
	namespace RekoAnimation {
		enum {
			BATTLE_CHARACTER_REQUIRED_ANIMATION,
			AttackBeam
		};
	}

	class Reko : public BattleCharacter {
	private:
		static constexpr int
			DEFAULT_MAX_HP = 500,
			DEFAULT_MAX_MP = 250,
			DEFAULT_ATK = 10,
			DEFAULT_DEF = 5,
			DEFAULT_SPEED = 10;
		SharedPtr<AnimatedSprite2D> _alphaAnim;
	protected:
		void _Load() override;
	};
}