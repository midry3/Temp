#pragma once
#include <Scene.h>
#include <Scene/Texture/AnimatedSprite2D.h>

#define BATTLE_CHARACTER_REQUIRED_ANIMATION Idle, Attack, Damage, Death

namespace Game
{
	struct BattleCharacterAnimationInfo {
		int animation;
		Path file;
	};

	struct BattleCharacterInfo {
		String screenName;
		std::vector<BattleCharacterAnimationInfo> animations;
	};

	struct BattleCharacterStats {
		int maxHP;
		int maxMP;
		int atk;
		int def;
		int speed;
	};

	struct BattleCharacterState {
		String screenName;
		int maxHP, currentHP;
		int maxMP, currentMP;
		int atk;
		int def;
		int speed;
	};

	class BattleCharacter : public Scene {
	private:
		BattleCharacterInfo _info;
		BattleCharacterStats _stats;
		BattleCharacterState _currentState;
		SharedPtr<AnimatedSprite2D> _sprite;
	protected:
		void _Load() override;

		void Initialize();
		void SetInfo(const BattleCharacterInfo info) { _info = info; }
		void SetStats(const BattleCharacterStats stats) { _stats = stats; }
		void SetState(const BattleCharacterState state) { _currentState = state; }
	public:
		void PlayAnimation(int animation, bool loop = false);
		void ApplyDamage(int damage);
		[[nodiscard]] int GetCurrentHP() const { return _currentState.currentHP; }
		[[nodiscard]] bool IsDead() const { return _currentState.currentHP <= 0; }
		[[nodiscard]] const BattleCharacterState& GetState() const { return _currentState; }
	};
}