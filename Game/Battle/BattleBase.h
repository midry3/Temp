#pragma once
#include <Scene.h>
#include "BattleCharacter/BattleCharacter.h"
#include "BattleUI.h"
#include "SkillSelect.h"
#include "BattleAttack.h"
#include "BattleBackground.h"
#include "Hud.h"
#include "UsingSkillPanel.h"
#include <Event/StateMachine.h>

namespace Game {
	class BattleBase : public Scene {
	private:
		enum State {
			Opening,
			Command,
			Direction,
			Won,
			Lost
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
		SharedPtr<AnimatedUI> _usingSkillView;
		SharedPtr<UsingSkillPanel> _allySkillPanel;
		SharedPtr<SkillSelect> _skillSelect;
		SharedPtr<BattleAttack> _attack;
		SharedPtr<BattleBackground> _background;
		SharedPtr<Hud> _hud;

		void _Load() override;
		void StartAttack();
		bool NextCharaExists();
		bool IsDefeatedAllEnemies();
	public:
		void PutCharacters();
		// 戦闘開始演出中に裏で進める事前ロード（敵味方全キャラの全スキル分）
		void QueueBattlePreload();
		bool ProcessBattlePreload();
		void SetAllies(std::vector<SharedPtr<BattleCharacter>> allies) { _allies = allies; }
		void AddAlly(SharedPtr<BattleCharacter> ally) { _allies.push_back(ally); }
		void SetEnemies(std::vector<SharedPtr<BattleCharacter>> enemies) { _enemies = enemies; }
		void AddEnemy(SharedPtr<BattleCharacter> enemy) { _enemies.push_back(enemy); }
	};
}
