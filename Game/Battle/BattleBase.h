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
#include <map>
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
			ALLIES_X_2 = 300,
			ENEMIES_X = 1000,
			CHARACTER_Y = 550,
			CHARACTER_GRID_HEIGHT = 500;

		// 味方の横一列フォーメーション（間隔と移動アニメーション時間、暗さ）
		static constexpr double ALLY_SPACING = 160.0;
		static constexpr double FORMATION_ANIMATION_TIME = 0.45;
		static constexpr int FORMATION_DIM_VALUE = 128;

		// 攻撃演出中、攻撃者と対象以外にかける薄黒の割合（元の色味に掛ける）
		static constexpr double ATTACK_DIM_RATE = 0.5;
		// 薄黒を戻すために控える、演出前の各キャラクターの色味
		std::map<BattleCharacter*, Vector3D> _attackDimRestore;

		StateMachine _state;
		std::vector<SkillInfo> _selectedSkills;
		std::vector<std::vector<SharedPtr<BattleCharacter>>> _selectedTargets;
		// _selectedSkills と同じ順序で、選択した味方の _allies 上のインデックス（死亡キャラは選択を飛ばすため）
		std::vector<int> _selectedAllyIndices;
		// _selectedSkills と同じ順序で、「何もしない」を選んだかどうか
		std::vector<bool> _selectedPasses;
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

		// 味方の並び（左→右。最後尾が最前列）
		std::vector<SharedPtr<BattleCharacter>> _allyFormation;
		SharedPtr<Scene> _alliesRow;
		// 最前列のX座標（通常は ALLIES_X、敵の攻撃時は ALLIES_X_2）
		double _formationFrontX = ALLIES_X;

		std::vector<SharedPtr<BattleCharacter>> GetTargetCandidates(const SharedPtr<BattleCharacter>& actor, const SkillInfo& skill) const;
		void AdvanceSelection();
		void GoBackSelection();
		void UpdateBackButton();
		void ShowTargetSelect(const SkillInfo& skill, const std::vector<SharedPtr<BattleCharacter>>& candidates);
		void BuildActionQueue();
		void ExecuteNextAction();
		void SetAllyFront(const SharedPtr<BattleCharacter>& character);
		void ApplyAllyFormation(bool animate);
		void ApplyAllyDimming();
		void AdvanceAllyFormationAfterAction();
		void ApplyAttackDimming(const SharedPtr<BattleCharacter>& actor, const std::vector<SharedPtr<BattleCharacter>>& targets);
		void ClearAttackDimming();
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
