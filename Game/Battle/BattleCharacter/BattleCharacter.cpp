#include "BattleCharacter.h"
#include "DamagedEffect.h"

namespace Game {
	void BattleCharacter::_Load() {
		_sprite = CreateScene<AnimatedSprite2D>();
		_sprite->AnimationFinished.Connect([this](int animation) {
			AnimationFinished.Emit(animation);
			});
		_powerUpEffect = _sprite->CreateScene<EffekseerEmitter2D>();
		_powerUpEffect->SetEffect("common_effect/powerup.efk");

		_powerUpSoundPlayer = _sprite->CreateScene<SoundPlayer>();
		_powerUpSoundPlayer->SetSound(LoadAsset<Sound>("sound/one_shot/status_up_1.mp3"))
			->SetVolume(0.7f);

		_sprite->AnimationChanged += [this](int anim) {
			const auto& s = _sprite->GetRawSize();
			_powerUpEffect->SetPosition({s.x/2, s.y/1.5});
			UpdateSpriteAlignment();
			};
		_sprite->FrameChanged.Connect([this](int frame) {
			UpdateSpriteAlignment();
			});
	}

	// スプライトの表示位置を、テクスチャの左上ではなく下端が
	// キャラの位置に合うようずらす（position 自体は動かさない）
	void BattleCharacter::UpdateSpriteAlignment() {
		if (not _sprite) return;
		const auto frameSize = _sprite->GetCurrentFrameSize();
		_sprite->offset.y = -frameSize.y * _sprite->scale.y;
	}

	namespace {
		void ApplyModulateRecursive(Scene* scene, const Vector3D& value) {
			if (not scene) return;
			scene->modulate = value;
			for (const auto& child : scene->GetChildScenes()) {
				ApplyModulateRecursive(child.get(), value);
			}
		}
	}

	void BattleCharacter::SetModulateAll(const Vector3D& value) {
		ApplyModulateRecursive(this, value);
	}

	// 被弾時の赤点滅（modulate を赤と白で数回切り替える）
	void BattleCharacter::PlayDamagedFlash() {
		const int serial = ++_damagedFlashSerial;
		SetModulateAll(Vector3D(255, 0, 0));
		for (int i = 1; i <= 5; ++i) {
			const bool toRed = i % 2 == 0;
			CreateTimer(0.075 * i, [this, serial, toRed]() {
				if (serial != _damagedFlashSerial) return;
				SetModulateAll(toRed ? Vector3D(255, 0, 0) : Vector3D(255, 255, 255));
				});
		}
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

	void BattleCharacter::SetStats(const BattleCharacterStats stats) {
		_info.stats = stats;
		_info.stats.maxHP = static_cast<int>(stats.maxHP * _statsScale);
		_info.stats.atk = static_cast<int>(stats.atk * _statsScale);
		_info.stats.def = static_cast<int>(stats.def * _statsScale);
		_info.stats.mdef = static_cast<int>(stats.mdef * _statsScale);
		_info.stats.speed = static_cast<int>(stats.speed * _statsScale);
		_info.currentState.currentHP = static_cast<int>(stats.maxHP * _statsScale);

		_info.currentState.atk = static_cast<int>(stats.atk * _statsScale);
		_info.currentState.def = static_cast<int>(stats.def * _statsScale);
		_info.currentState.mdef = static_cast<int>(stats.mdef * _statsScale);
		_info.currentState.speed = static_cast<int>(stats.speed * _statsScale);
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
		const bool wasDead = IsDead();
		const int appliedDamage = Max(0, damage);
		_info.currentState.currentHP = Max(0, _info.currentState.currentHP - appliedDamage);
		if (0 < appliedDamage) {
			Damaged.Emit();
			PlayDamagedFlash();
		}
		if (not wasDead && IsDead()) Died.Emit();
		if (IsDead()) {
			// 死亡演出
		}
		else {
			// 被弾演出
		}
		CreateScene<DamagedEffect>()->SetDamageAmount(appliedDamage);
		StatChanged.Emit();
	}

	void BattleCharacter::UseMP(int amount) {
		_info.currentState.currentMP = Max(0, _info.currentState.currentMP - amount);
		StatChanged.Emit();
	}

	bool BattleCharacter::CanUseSkill(int mp) {
		return mp <= _info.currentState.currentMP;
	}

	void BattleCharacter::ModifyStat(const std::string& stat, int delta) {
		auto& s = _info.currentState;
		const auto& base = _info.stats;
		if (stat == "CurrentHP") {
			DebuggerWriteLine("BattleCharacter::ModifyStat: CurrentHP changed by " + std::to_string(delta));
			s.currentHP = Clamp(s.currentHP + delta, 0, base.maxHP);
			if (0 < delta) {
				_powerUpSoundPlayer->Play();
			}
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
				_powerUpSoundPlayer->Play();
			}
		}

		StatChanged.Emit();
	}
}
