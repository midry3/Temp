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
	};
}
