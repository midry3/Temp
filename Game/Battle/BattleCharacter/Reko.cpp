#include "Reko.h"

namespace Game {
	void Reko::_Load() {
		BattleCharacter::_Load();
		_alphaAnim = CreateScene<RekoAlpha>();
		_alphaAnim->AddPart({
			RekoAlphaPartGroup::Torso,
			"chara/reko/alpha_idle.png",
			{ 150, -150 }
			});
		_sprite->AnimationChanged += [this](int animation) {
			if (animation == RekoAnimation::AttackBeam) {
				_alphaAnim->SetBeam();
			}
			else if (animation == RekoAnimation::OpenPC && not _sprite->IsReversed()) {
				_sprite->Play(RekoAnimation::UsingPC, true);
			}
			else {
				_alphaAnim->SetIdle();
			}
			};

		SetInfo({
			"レコ",
			{
				{ RekoAnimation::Idle, "chara/reko/idle.png" },
				/*
				{ RekoAnimation::Attack, "chara/reko/reko_attack.png" },
				{ RekoAnimation::Damage, "chara/reko/reko_damage.png" },
				{ RekoAnimation::Death, "chara/reko/reko_death.png" }
				*/
				{ RekoAnimation::AttackBeam, "chara/reko/attack.png" },
				{ RekoAnimation::OpenPC, "chara/reko/pc_open.png" },
				{ RekoAnimation::UsingPC, "chara/reko/pc.png" }
			}
			});
		SetStats({
			100, 50, 10, 5, 7
			});
		SetSkills({
			{ "ビーム", "special", "fire", "chara/reko/attack/beam.lua" },
			{ "Fire", "special", "fire", "battle/fire.lua" },
			{ "セキュリティアップデート", "special", "water", "chara/reko/attack/security_update.lua" },
			{ "ソード", "special", "nature", "chara/reko/attack/sword.lua" }
			});

		Initialize();
	}
}