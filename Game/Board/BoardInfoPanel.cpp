#include "BoardInfoPanel.h"
#include <Scene/Text/TextLabel.h>
#include <algorithm>

namespace Game {
	namespace {
		// パネル全体（子シーンを含む）の色味に割合を掛ける。
		// HPバーなどは modulate で色付けされているため、上書きせずに暗くする
		void MultiplyModulateAll(const SharedPtr<Scene>& scene, double rate) {
			if (not scene) return;
			const auto base = scene->modulate;
			scene->modulate = Vector3D(base.x * rate, base.y * rate, base.z * rate);
			for (const auto& child : scene->GetChildScenes()) {
				MultiplyModulateAll(child, rate);
			}
		}
	}

	void BoardInfoPanel::_Load() {
		_background = CreateScene<ColorRect>();
		_background->SetColor({ 24, 28, 38 })
			->SetAlpha(255);

		const auto& allyHeading = CreateScene<TextLabel>();
		allyHeading->BeginSetting()
			->SetFontSize(22)
			->SetForegroundColor({ 255, 255, 255 })
			->EndSetting()
			->SetText("味方ステータス")
			->SetPosition({ 12, 6 });

		// 味方パネル (196x124) を 2x2 で並べる
		_allyGrid = CreateScene<GridContainer>();
		_allyGrid->SetSpace({ BoardStatusPanel::PANEL_WIDTH + 8, BoardStatusPanel::PANEL_HEIGHT + 6 })
			->SetMatrixSize({ 2, 2 })
			->SetPosition({ 8, 34 });

		const auto& enemyHeading = CreateScene<TextLabel>();
		enemyHeading->BeginSetting()
			->SetFontSize(22)
			->SetForegroundColor({ 255, 255, 255 })
			->EndSetting()
			->SetText("敵情報")
			->SetPosition({ 12, 306 });

		_enemyGrid = CreateScene<GridContainer>();
		_enemyGrid->SetSpace({ 0, BoardEnemyStatusPanel::PANEL_HEIGHT + 6 })
			->SetMatrixSize({ 1, 3 })
			->SetPosition({ 8, 336 });
	}

	void BoardInfoPanel::SetPanelSize(double width, double height) {
		_background->SetSize({ width, height });
	}

	void BoardInfoPanel::SetAllies(const std::vector<SharedPtr<BattleCharacter>>& allies) {
		_allyCharacters = allies;
		RebuildAllyPanels();
	}

	void BoardInfoPanel::AddDefeatedAlly(const SharedPtr<BattleCharacter>& character) {
		if (not character) return;
		if (std::find(_defeatedAllies.begin(), _defeatedAllies.end(), character) != _defeatedAllies.end()) return;
		_defeatedAllies.push_back(character);
		RebuildAllyPanels();
	}

	void BoardInfoPanel::RebuildAllyPanels() {
		for (const auto& panel : _allyPanels) {
			_allyGrid->DeleteScene(panel);
		}
		_allyPanels.clear();
		for (const auto& character : _allyCharacters) {
			if (not character) continue;
			if (std::find(_defeatedAllies.begin(), _defeatedAllies.end(), character) != _defeatedAllies.end()) continue;
			const auto& panel = _allyGrid->CreateScene<BoardStatusPanel>();
			panel->SetCharacter(character);
			_allyPanels.push_back(panel);
		}
		// 倒れた味方は薄黒のパネルとして末尾に残す
		for (const auto& character : _defeatedAllies) {
			const auto& panel = _allyGrid->CreateScene<BoardStatusPanel>();
			panel->SetCharacter(character);
			panel->UpdateView();
			MultiplyModulateAll(panel, PANEL_DEAD_DIM_RATE);
			_allyPanels.push_back(panel);
		}
	}

	void BoardInfoPanel::ShowEnemies(const std::vector<SharedPtr<BattleCharacter>>& enemies) {
		ClearEnemies();
		for (const auto& character : enemies) {
			if (not character) continue;
			const auto& panel = _enemyGrid->CreateScene<BoardEnemyStatusPanel>();
			panel->SetCharacter(character);
			_enemyPanels.push_back(panel);
		}
	}

	void BoardInfoPanel::ClearEnemies() {
		for (const auto& panel : _enemyPanels) {
			_enemyGrid->DeleteScene(panel);
		}
		_enemyPanels.clear();
	}

	void BoardInfoPanel::UpdateView() {
		for (const auto& panel : _allyPanels) {
			panel->UpdateView();
		}
		for (const auto& panel : _enemyPanels) {
			panel->UpdateView();
		}
	}
}
