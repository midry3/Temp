#pragma once
#include <Scene.h>
#include "StatusPanel.h"
#include "EnemyStatsPanel.h"
#include <Scene/Container/Grid/GridContainer.h>
#include "BattleCharacter/BattleCharacter.h"
#include <optional>
#include <utility>

namespace Game {
	class Hud : public Scene {
	private:
		std::vector<SharedPtr<BattleCharacter>> _allies, _enemies;
		std::vector<SharedPtr<StatusPanel>> _allyStatusPanels;
		std::vector<SharedPtr<EnemyStatsPanel>> _enemyStatusPanels;
		SharedPtr<GridContainer> _allyStatusGrid, _enemyStatusGrid;
	protected:
		void _Load() override;
	public:
		void SetAllies(const std::vector<SharedPtr<BattleCharacter>>& allies);
		void SetEnemies(const std::vector<SharedPtr<BattleCharacter>>& enemies);
		void UpdateView();
		// キャラの情報パネルの表示矩形（左上座標と大きさ）。表示がなければ nullopt。
		std::optional<std::pair<Vector2D, Vector2D>> GetCharacterPanelRect(const SharedPtr<BattleCharacter>& character) const;
	};
}