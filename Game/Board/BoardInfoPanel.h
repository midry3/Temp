#pragma once
#include <Scene.h>
#include <Scene/Container/Grid/GridContainer.h>
#include <vector>
#include "../Battle/StatusPanel.h"
#include "../Battle/EnemyStatsPanel.h"

namespace Game {
	class BattleCharacter;

	// 盤面右側の情報パネル。上半分に味方4人のステータスを 2x2 で表示し、
	// 下半分にクリックしたマスの敵情報を表示する。内部レイアウトは設計サイズ
	// （WIDTH x 720）で組み、盤面の右に残ったスペースに合わせて拡縮される。
	class BoardInfoPanel : public Scene {
	public:
		static constexpr int WIDTH = 700;
	private:
		SharedPtr<GridContainer> _allyGrid;
		SharedPtr<GridContainer> _enemyGrid;
		std::vector<SharedPtr<StatusPanel>> _allyPanels;
		std::vector<SharedPtr<EnemyStatsPanel>> _enemyPanels;
	protected:
		void _Load() override;
	public:
		void SetAllies(const std::vector<SharedPtr<BattleCharacter>>& allies);
		void ShowEnemies(const std::vector<SharedPtr<BattleCharacter>>& enemies);
		void ClearEnemies();
		void UpdateView();
	};
}
