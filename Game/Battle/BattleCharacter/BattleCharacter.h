#pragma once
#include <Scene.h>
#include <Scene/Texture/AnimatedSprite2D.h>
#include <Effekseer/EffekseerEmitter.h>
#include <Scene/Sound/SoundPlayer.h>
#include "../SkillInfo.h"
#include <optional>
#include <string>

#define BATTLE_CHARACTER_REQUIRED_ANIMATION Idle, Attack, Damage, Death

namespace Game
{
	struct BattleCharacterAnimationInfo {
		int animation;
		Path file;
	};

	struct BattleCharacterStats {
		int maxHP;
		int maxMP;
		int atk;
		int def;
		int mdef;
		int speed;
	};

	struct BattleCharacterState {
		int currentHP;
		int currentMP;
		int atk;
		int def;
		int mdef;
		int speed;
	};

	struct BattleCharacterInfo {
		String screenName;
		Path icon;
		BattleCharacterStats stats;
		BattleCharacterState currentState;
		std::vector<BattleCharacterAnimationInfo> animations;
		unsigned int themeColor = CoreBackend::Drawing::Color(100, 255, 100);
	};

	class BattleCharacter : public Scene {
	private:
		BattleCharacterInfo _info;
		std::vector<SkillInfo> _skills;

		double _statsScale = 1.0;
	protected:
		SharedPtr<AnimatedSprite2D> _sprite;
		SharedPtr<EffekseerEmitter2D> _powerUpEffect;
		SharedPtr<SoundPlayer> _powerUpSoundPlayer;
		void _Load() override;

		void Initialize();
		void SetInfo(const BattleCharacterInfo info) { _info = info; }
		void SetStats(const BattleCharacterStats stats);
		void SetState(const BattleCharacterState state) { _info.currentState = state; }
		void SetSkills(const std::vector<SkillInfo>& skills) { _skills = skills; }
		void SetStatsScale(double scale) { _statsScale = scale; }
	public:
		Signal<> StatChanged;
		Signal<> AttackStarted;
		Signal<> Damaged;
		Signal<> Died;
		Signal<int> AnimationFinished;
		void PlayAnimation(int animation, std::optional<bool> loop = std::nullopt, bool reverse = false);
		[[nodiscard]] Vector2D* GetAnimationPositionPointer();
		void ApplyDamage(int damage);
		void UseMP(int amount);
		[[nodiscard]] bool CanUseSkill(int mp);
		void ModifyStat(const std::string& stat, int delta);
		// Board 側で保持する永続状態を復元する（戦闘開始時の状態引き継ぎ用）
		void RestoreState(const BattleCharacterState& state) { _info.currentState = state; }
		[[nodiscard]] int GetCurrentHP() const { return _info.currentState.currentHP; }
		[[nodiscard]] bool IsDead() const { return _info.currentState.currentHP <= 0; }
		[[nodiscard]] const BattleCharacterInfo& GetInfo() const { return _info; }
		[[nodiscard]] const BattleCharacterState& GetState() const { return _info.currentState; }
		[[nodiscard]] const std::vector<SkillInfo>& GetSkills() const { return _skills; }
	};
}
