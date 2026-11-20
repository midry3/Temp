#include "Reko.h"

namespace Game {
	void Reko::_Load() {
		BattleCharacter::_Load();
		_alphaAnim = CreateScene<AnimatedSprite2D>();
		_alphaAnim
			->AddAnimation(RekoAnimation::Idle, LoadAsset<SpriteSheet>("chara/reko/alpha_idle.png"))
			->AddAnimation(RekoAnimation::AttackBeam, LoadAsset<SpriteSheet>("chara/reko/alpha_beam.png"))
			->SetPosition({ 150, -150 });
		_sprite->AnimationChanged += [this](int animation) {
			_alphaAnim->Play(animation);
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
				{ RekoAnimation::AttackBeam, "chara/reko/attack.png" }
			}
			});
		SetStats({
			100, 50, 10, 5, 7
			});
		SetSkills({
			{ "ビーム", "special", "fire", "chara/reko/attack/beam.lua" },
			{ "Fire", "special", "fire", "battle/fire.lua" },
			{ "Ice", "special", "water", "battle/ice.lua" },
			{ "Heal", "special", "nature", "battle/heal.lua" }
			});

		Initialize();
	}
}