#include "RekoAlpha.h"

namespace Game {
	void RekoAlpha::SetIdle() {
		for (const auto& torso : GetParts(RekoAlphaPartGroup::Torso)) {
			torso->AddAnimation(0, LoadAsset<SpriteSheet>("chara/reko/alpha_idle.png"));
			torso->Play(0);
		}
	}

	void RekoAlpha::SetBeam() {
		for (const auto& torso : GetParts(RekoAlphaPartGroup::Torso)) {
			torso->AddAnimation(1, LoadAsset<SpriteSheet>("chara/reko/alpha_beam.png"));
			torso->Play(1);
		}
	}
}
