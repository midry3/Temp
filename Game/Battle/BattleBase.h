#pragma once
#include <Scene.h>
#include "BattleCharacter/BattleCharacter.h"
#include "BattleUI.h"
#include "SkillSelect.h"
#include "BattleAttack.h"

namespace Game {
	class BattleBase : public Scene {
	private:
		static constexpr int
			ALLIES_X = 300,
			ENEMIES_X = 1000,
			CHARACTER_Y = 280,
			CHARACTER_GRID_HEIGHT = 500;
	protected:
		std::vector<SharedPtr<BattleCharacter>> _allies, _enemies;
		SharedPtr<BattleUI> _battleUI;
		SharedPtr<SkillSelect> _skillSelect;
		SharedPtr<BattleAttack> _attack;

		void _Load() override;
		void PutCharacters();
		void StartAttack(const SkillInfo& skill);
	public:
		void SetAllies(std::vector<SharedPtr<BattleCharacter>> allies) { _allies = allies; }
		void AddAlly(SharedPtr<BattleCharacter> ally) { _allies.push_back(ally); }
		void SetEnemies(std::vector<SharedPtr<BattleCharacter>> enemies) { _enemies = enemies; }
		void AddEnemy(SharedPtr<BattleCharacter> enemy) { _enemies.push_back(enemy); }
	};
}