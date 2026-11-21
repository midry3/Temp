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

	void RekoAlpha::SetSwordPrepare(bool reversed) {
		for (const auto& torso : GetParts(RekoAlphaPartGroup::Torso)) {
			torso->AddAnimation(2, LoadAsset<SpriteSheet>("chara/reko/alpha_sword_prepare.png"));
			torso->Play(2, false, reversed);
		}
	}

	void RekoAlpha::SetSwordAttack(bool reversed) {
		for (const auto& torso : GetParts(RekoAlphaPartGroup::Torso)) {
			torso->AddAnimation(3, LoadAsset<SpriteSheet>("chara/reko/alpha_sword_attack.png"));
			torso->Play(3, false, reversed);
		}
	}
}
