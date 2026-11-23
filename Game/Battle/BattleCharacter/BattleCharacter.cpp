#include "BattleCharacter.h"

namespace Game {
	void BattleCharacter::_Load() {
		_sprite = CreateScene<AnimatedSprite2D>();
		_sprite->AnimationFinished.Connect([this](int animation) {
			AnimationFinished.Emit(animation);
			});
		_powerUpEffect = _sprite->CreateScene<EffekseerEmitter2D>();
		_powerUpEffect->SetEffect("common_effect/powerup.efk");

		_sprite->AnimationChanged += [this](int anim) {
			const auto& s = _sprite->GetRawSize();
			_powerUpEffect->SetPosition({s.x/2, s.y/1.5});
			};
	}

	void BattleCharacter::Initialize() {
		for (const auto& animationInfo : _info.animations) {
			_sprite->AddAnimation(animationInfo.animation, LoadAsset<SpriteSheet>(animationInfo.file));
		}

		_info.currentState = {
			_info.stats.maxHP,
			_info.stats.maxMP,
			_info.stats.atk,
			_info.stats.def,
			_info.stats.mdef,
			_info.stats.speed
		};

		_sprite->Play(0);
	}

	void BattleCharacter::PlayAnimation(int animation, std::optional<bool> loop, bool reverse) {
		if (_sprite) {
			_sprite->Play(animation, loop, reverse);
		}
	}

	Vector2D* BattleCharacter::GetAnimationPositionPointer() {
		return _sprite ? _sprite->GetPositionPointer() : nullptr;
	}

	void BattleCharacter::ApplyDamage(int damage) {
		const int appliedDamage = damage > 0 ? damage : 0;
		_info.currentState.currentHP = _info.currentState.currentHP > appliedDamage
			? _info.currentState.currentHP - appliedDamage : 0;
		if (IsDead()) {
			PlayAnimation(3);
		}
		else {
			PlayAnimation(2);
		}
	}

	void BattleCharacter::ModifyStat(const std::string& stat, int delta) {
		auto& s = _info.currentState;
		const auto& base = _info.stats;
		if (stat == "CurrentHP") {
			s.currentHP = Clamp(s.currentHP + delta, 0, base.maxHP);
		}
		else {
			if (stat == "ATK") {
				s.atk = Max(0, s.atk + delta);
			}
			else if (stat == "DEF") {
				s.def = Max(0, s.def + delta);
			}
			else if (stat == "MDEF") {
				s.mdef = Max(0, s.mdef + delta);
			}
			else if (stat == "SPD") {
				s.speed = Max(0, s.speed + delta);
			}

			if (0 < delta) {
				_powerUpEffect->Play();
			}
		}

		StatChanged.Emit();
	}
}
