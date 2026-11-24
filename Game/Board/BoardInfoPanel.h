#pragma once
#include <Scene.h>
#include <Scene/Container/Grid/GridContainer.h>
#include <Scene/Shape/ColorRect.h>
#include <vector>
#include "BoardStatusPanel.h"
#include "BoardEnemyStatusPanel.h"

namespace Game {
	class BattleCharacter;

	// 盤面右側の情報パネル。上半分に味方4人のステータスを 2x2 で表示し、
	// 下半分にクリックしたマスの敵情報を表示する。表示は実寸（スケール調整なし）で、
	// 幅と高さは BoardBase のレイアウト計算から SetPanelSize で受け取る。
	class BoardInfoPanel : public Scene {
	private:
		// 倒れた味方のパネルにかける薄黒
		static constexpr int PANEL_DEAD_DIM_VALUE = 128;

		SharedPtr<ColorRect> _background;
		SharedPtr<GridContainer> _allyGrid;
		SharedPtr<GridContainer> _enemyGrid;
		std::vector<SharedPtr<BoardStatusPanel>> _allyPanels;
		std::vector<SharedPtr<BoardEnemyStatusPanel>> _enemyPanels;
		std::vector<SharedPtr<BattleCharacter>> _allyCharacters;
		std::vector<SharedPtr<BattleCharacter>> _defeatedAllies;

		void RebuildAllyPanels();
	protected:
		void _Load() override;
	public:
		void SetPanelSize(double width, double height);
		void SetAllies(const std::vector<SharedPtr<BattleCharacter>>& allies);
		// 倒れて盤面から消えた味方を、薄黒のパネルとして残す
		void AddDefeatedAlly(const SharedPtr<BattleCharacter>& character);
		void ShowEnemies(const std::vector<SharedPtr<BattleCharacter>>& enemies);
		void ClearEnemies();
		void UpdateView();
	};
}
