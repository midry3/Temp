#pragma once
#include "CompositeSprite.h"

namespace Game {
	namespace RekoAlphaPartGroup {
		enum {
			Head,
			Torso,
			Legs
		};
	}

	class RekoAlpha : public CompositeSprite {
	public:
		void SetIdle();
		void SetBeam();
		void SetSwordPrepare(bool reversed = false);
		void SetSwordAttack(bool reversed = false);
	};
}
