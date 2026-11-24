#include "Hud.h"

namespace Game {
	void Hud::_Load() {
		_allyStatusGrid = CreateScene<GridContainer>();
		_allyStatusGrid->SetSpace({ 0, 115 })
			->SetPosition({ 20, 20 });

		_enemyStatusGrid = CreateScene<GridContainer>();
		_enemyStatusGrid->SetSpace({ 0, 110 })
			->SetPosition({ 900, 20 });
	}

	void Hud::SetAllies(const std::vector<SharedPtr<BattleCharacter>>& allies) {
		_allies = allies;

		_allyStatusGrid->SetMatrixSize({ 1, static_cast<double>(_allies.size()) });
		_allyStatusPanels.clear();
		for (const auto& character : _allies) {
			const auto& panel = _allyStatusGrid->CreateScene<StatusPanel>();
			panel->SetCharacter(character);
			_allyStatusPanels.push_back(panel);
		}
	}

	void Hud::SetEnemies(const std::vector<SharedPtr<BattleCharacter>>& enemies) {
		_enemies = enemies;
		_enemyStatusGrid->SetMatrixSize({ 1, static_cast<double>(_enemies.size()) });
		_enemyStatusPanels.clear();
		for (const auto& character : _enemies) {
			const auto& panel = _enemyStatusGrid->CreateScene<EnemyStatsPanel>();
			panel->SetCharacter(character);
			_enemyStatusPanels.push_back(panel);
		}
	}

	std::optional<std::pair<Vector2D, Vector2D>> Hud::GetCharacterPanelRect(const SharedPtr<BattleCharacter>& character) const {
		// パネル本体の位置と大きさは StatusPanel / EnemyStatsPanel の定義に合わせる
		for (size_t i = 0; i < _allies.size() && i < _allyStatusPanels.size(); ++i) {
			if (_allies[i] == character && _allyStatusPanels[i]) {
				const auto position = _allyStatusPanels[i]->GetGlobalPosition() + Vector2D(20, 20);
				return std::make_pair(position, Vector2D(320, 106));
			}
		}
		for (size_t i = 0; i < _enemies.size() && i < _enemyStatusPanels.size(); ++i) {
			if (_enemies[i] == character && _enemyStatusPanels[i]) {
				const auto position = _enemyStatusPanels[i]->GetGlobalPosition() + Vector2D(10, 10);
				return std::make_pair(position, Vector2D(380, 100));
			}
		}
		return std::nullopt;
	}

	void Hud::UpdateView() {
		for (const auto& panel : _allyStatusPanels) {
			panel->UpdateView();
		}
		for (const auto& panel : _enemyStatusPanels) {
			panel->UpdateView();
		}
	}
}