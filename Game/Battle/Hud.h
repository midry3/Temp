#pragma once
#include <Scene.h>
#include "StatusPanel.h"
#include <Scene/Container/Grid/GridContainer.h>
#include "BattleCharacter/BattleCharacter.h"

namespace Game {
	class Hud : public Scene {
	private:
		std::vector<SharedPtr<BattleCharacter>> _allies, _enemies;
		std::vector<SharedPtr<StatusPanel>> _allyStatusPanels, _enemyStatusPanels;
		SharedPtr<GridContainer> _allyStatusGrid, _enemyStatusGrid;
	protected:
		void _Load() override;
	public:
		void SetAllies(const std::vector<SharedPtr<BattleCharacter>>& allies);
		void SetEnemies(const std::vector<SharedPtr<BattleCharacter>>& enemies);
		void UpdateView();
	};
}