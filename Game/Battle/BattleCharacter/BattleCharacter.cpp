#include "BattleCharacter.h"

namespace Game {
	void BattleCharacter::_Load() {
		_sprite = CreateScene<AnimatedSprite2D>();
	}

	void BattleCharacter::Initialize() {
		for (const auto& animationInfo : _info.animations) {
			_sprite->AddAnimation(animationInfo.animation, LoadAsset<SpriteSheet>(animationInfo.file));
		}

		_currentState = {
			_info.screenName,
			_stats.maxHP, _stats.maxHP,
			_stats.maxMP, _stats.maxMP,
			_stats.atk,
			_stats.def,
			_stats.speed
		};

		_sprite->Play(0);
	}

	void BattleCharacter::PlayAnimation(int animation, bool loop) {
		if (_sprite) {
			_sprite->Play(animation, loop);
		}
	}

	void BattleCharacter::ApplyDamage(int damage) {
		const int appliedDamage = damage > 0 ? damage : 0;
		_currentState.currentHP = _currentState.currentHP > appliedDamage
			? _currentState.currentHP - appliedDamage : 0;
		if (IsDead()) {
			PlayAnimation(3);
		}
		else {
			PlayAnimation(2);
		}
	}
}