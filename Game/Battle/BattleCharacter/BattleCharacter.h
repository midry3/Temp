#pragma once
#include <Scene.h>
#include <Scene/Texture/AnimatedSprite2D.h>
#include <Effekseer/EffekseerEmitter.h>
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
	protected:
		SharedPtr<AnimatedSprite2D> _sprite;
		SharedPtr<EffekseerEmitter2D> _powerUpEffect;

		void _Load() override;

		void Initialize();
		void SetInfo(const BattleCharacterInfo info) { _info = info; }
		void SetStats(const BattleCharacterStats stats) { _info.stats = stats; }
		void SetState(const BattleCharacterState state) { _info.currentState = state; }
		void SetSkills(const std::vector<SkillInfo>& skills) { _skills = skills; }
	public:
		Signal<> StatChanged;
		Signal<int> AnimationFinished;
		void PlayAnimation(int animation, std::optional<bool> loop = std::nullopt, bool reverse = false);
		[[nodiscard]] Vector2D* GetAnimationPositionPointer();
		void ApplyDamage(int damage);
		void ModifyStat(const std::string& stat, int delta);
		[[nodiscard]] int GetCurrentHP() const { return _info.currentState.currentHP; }
		[[nodiscard]] bool IsDead() const { return _info.currentState.currentHP <= 0; }
		[[nodiscard]] const BattleCharacterInfo& GetInfo() const { return _info; }
		[[nodiscard]] const BattleCharacterState& GetState() const { return _info.currentState; }
		[[nodiscard]] const std::vector<SkillInfo>& GetSkills() const { return _skills; }
	};
}
