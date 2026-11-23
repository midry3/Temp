#pragma once
#include <Scene.h>
#include "BattleCharacter/BattleCharacter.h"
#include "BattleUI.h"
#include "SkillSelect.h"
#include "BattleAttack.h"
#include "BattleBackground.h"
#include "Hud.h"
#include <Event/StateMachine.h>

namespace Game {
	class BattleBase : public Scene {
	private:
		enum State {
			Opening,
			Command,
			Direction,
		};

		static constexpr int
			ALLIES_X = 100,
			ENEMIES_X = 1000,
			CHARACTER_Y = 280,
			CHARACTER_GRID_HEIGHT = 500;

		StateMachine _state;
		std::vector<SkillInfo> _selectedSkills;
		int _currentCharacterIndex = 0;
		bool _charactersPut = false;

	protected:
		std::vector<SharedPtr<BattleCharacter>> _allies, _enemies;
		SharedPtr<BattleUI> _battleUI;
		SharedPtr<SkillSelect> _skillSelect;
		SharedPtr<BattleAttack> _attack;
		SharedPtr<BattleBackground> _background;
		SharedPtr<Hud> _hud;

		void _Load() override;
		void StartAttack();
		bool NextCharaExists();
	public:
		void PutCharacters();
		void SetAllies(std::vector<SharedPtr<BattleCharacter>> allies) { _allies = allies; }
		void AddAlly(SharedPtr<BattleCharacter> ally) { _allies.push_back(ally); }
		void SetEnemies(std::vector<SharedPtr<BattleCharacter>> enemies) { _enemies = enemies; }
		void AddEnemy(SharedPtr<BattleCharacter> enemy) { _enemies.push_back(enemy); }
	};
}
