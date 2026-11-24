#pragma once
#include <Scene.h>
#include "BattleCharacter/BattleCharacter.h"
#include "BattleUI.h"
#include "SkillSelect.h"
#include "BattleAttack.h"
#include "BattleBackground.h"
#include "Hud.h"
#include "TargetSelect.h"
#include "UsingSkillPanel.h"
#include <Event/StateMachine.h>
#include <optional>

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
		std::vector<std::vector<SharedPtr<BattleCharacter>>> _selectedTargets;
		int _currentCharacterIndex = 0;
		bool _charactersPut = false;

		// 1ターン分の行動（味方の選択分と敵のAI分をまとめて行動順に並べる）
		struct QueuedAction {
			SharedPtr<BattleCharacter> actor;
			SkillInfo skill;
			std::vector<SharedPtr<BattleCharacter>> targets;
		};
		std::vector<QueuedAction> _actionQueue;
		size_t _actionIndex = 0;

		std::vector<SharedPtr<BattleCharacter>> GetTargetCandidates(const SharedPtr<BattleCharacter>& actor, const SkillInfo& skill) const;
		void AdvanceSelection();
		void ShowTargetSelect(const SkillInfo& skill, const std::vector<SharedPtr<BattleCharacter>>& candidates);
		void BuildActionQueue();
		void ExecuteNextAction();
		std::optional<SkillInfo> ChooseEnemySkill(const SharedPtr<BattleCharacter>& enemy) const;
		std::vector<SharedPtr<BattleCharacter>> ChooseEnemyTargets(const SharedPtr<BattleCharacter>& enemy, const SkillInfo& skill) const;
		bool IsEnemyCharacter(const SharedPtr<BattleCharacter>& character) const;

	protected:
		std::vector<SharedPtr<BattleCharacter>> _allies, _enemies;
		SharedPtr<BattleUI> _battleUI;
		SharedPtr<AnimatedUI> _usingSkillView;
		SharedPtr<UsingSkillPanel> _allySkillPanel;
		SharedPtr<SkillSelect> _skillSelect;
		SharedPtr<BattleAttack> _attack;
		SharedPtr<BattleBackground> _background;
		SharedPtr<Hud> _hud;
		SharedPtr<TargetSelect> _targetSelect;

		void _Load() override;
		bool NextCharaExists();
		bool IsDefeatedAllEnemies();
		bool IsDefeatedAllAllies();
	public:
		void PutCharacters();
		void SetAllies(std::vector<SharedPtr<BattleCharacter>> allies) { _allies = allies; }
		void AddAlly(SharedPtr<BattleCharacter> ally) { _allies.push_back(ally); }
		void SetEnemies(std::vector<SharedPtr<BattleCharacter>> enemies) { _enemies = enemies; }
		void AddEnemy(SharedPtr<BattleCharacter> enemy) { _enemies.push_back(enemy); }
	};
}
