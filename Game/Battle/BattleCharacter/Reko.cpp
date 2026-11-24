#include "Reko.h"

namespace Game {
	void Reko::OnFormationFront(bool isFront) {
		if (_alphaAnim) {
			_alphaAnim->alpha = isFront ? 255 : ALPHA_NOT_FRONT;
		}
	}

	void Reko::_Load() {
		BattleCharacter::_Load();
		_alphaAnim = CreateScene<RekoAlpha>();
		_alphaAnim->AddPart({
			RekoAlphaPartGroup::Torso,
			"chara/reko/alpha_idle.png",
			{ 50, -150 }
			});
		_alphaAnim->SetOffset({ 0, -300 });
		_sprite->AnimationChanged += [this](int animation) {
			if (animation == RekoAnimation::AttackBeam) {
				_alphaAnim->SetBeam();
			}
			else if (animation == RekoAnimation::OpenPC && not _sprite->IsReversed()) {
				_sprite->Play(RekoAnimation::UsingPC, true);
			}
			else if (animation == RekoAnimation::SwordPrepare) {
				_alphaAnim->SetSwordPrepare(_sprite->IsReversed());
			}
			else if (animation == RekoAnimation::SwordAttack) {
				_alphaAnim->SetSwordAttack(_sprite->IsReversed());
			}
			else {
				_alphaAnim->SetIdle();
			}
			};

		SetInfo({
			"レコ",
			"chara/reko/icon.png",
			{
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
				{ RekoAnimation::Idle, "chara/reko/idle.png" },
				/*
				{ RekoAnimation::Attack, "chara/reko/reko_attack.png" },
				{ RekoAnimation::Damage, "chara/reko/reko_damage.png" },
				{ RekoAnimation::Death, "chara/reko/reko_death.png" }
				*/
				{ RekoAnimation::AttackBeam, "chara/reko/attack.png" },
				{ RekoAnimation::OpenPC, "chara/reko/pc_open.png" },
				{ RekoAnimation::UsingPC, "chara/reko/pc.png" },
				{ RekoAnimation::SwordPrepare, "chara/reko/idle.png" },
				{ RekoAnimation::SwordAttack, "chara/reko/attack.png" },
			},
			CoreBackend::Drawing::Color(100, 255, 200)
			});
		SetSkills({
			{ "ビーム", "special", "fire", "chara/reko/attack/beam.lua", 20 },
			{ "Fire", "special", "fire", "battle/fire.lua" },
			{ "セキュリティアップデート", "special", "water", "chara/reko/attack/security_update.lua", 20, SkillTargetScope::Single, SkillTargetSide::Self },
			{ "ソード", "special", "nature", "chara/reko/attack/sword.lua", 50 }
			});

		Initialize();
	}
}