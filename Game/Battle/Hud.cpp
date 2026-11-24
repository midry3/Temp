#include "Hud.h"

namespace Game {
	namespace {
		// パネル全体（子シーンを含む）を列挙する
		void CollectScenes(const SharedPtr<Scene>& scene, std::vector<Scene*>& out) {
			if (not scene) return;
			out.push_back(scene.get());
			for (const auto& child : scene->GetChildScenes()) {
				CollectScenes(child, out);
			}
		}
	}

	void Hud::_Load() {
		_allyStatusGrid = CreateScene<GridContainer>();
		_allyStatusGrid->SetSpace({ 0, 115 })
			->SetPosition({ 20, 20 });

		_enemyStatusGrid = CreateScene<GridContainer>();
		_enemyStatusGrid->SetSpace({ 0, 110 })
			->SetPosition({ 900, 20 });
	}

	void Hud::SetAllies(const std::vector<SharedPtr<BattleCharacter>>& allies) {
		_panelDimRestore.clear();
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
		_panelDimRestore.clear();
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
		// 死亡したキャラクターのパネルは薄黒にする
		for (size_t i = 0; i < _allyStatusPanels.size(); ++i) {
			_allyStatusPanels[i]->UpdateView();
			const bool dead = i < _allies.size() && _allies[i] && _allies[i]->IsDead();
			ApplyPanelDim(_allyStatusPanels[i], dead);
		}
		for (size_t i = 0; i < _enemyStatusPanels.size(); ++i) {
			_enemyStatusPanels[i]->UpdateView();
			const bool dead = i < _enemies.size() && _enemies[i] && _enemies[i]->IsDead();
			ApplyPanelDim(_enemyStatusPanels[i], dead);
		}
	}

	void Hud::ApplyPanelDim(const SharedPtr<Scene>& panel, bool dim) {
		// HPバーなどは modulate で色付けされているため、白や灰色で上書きせず、
		// 元の色味を控えたうえで割合を掛けて暗くする（バーの色を保ったまま薄黒にする）
		std::vector<Scene*> nodes;
		CollectScenes(panel, nodes);
		for (auto* node : nodes) {
			if (dim) {
				if (_panelDimRestore.count(node) == 0) {
					_panelDimRestore[node] = node->modulate;
				}
				const auto base = _panelDimRestore[node];
				node->modulate = Vector3D(base.x * PANEL_DEAD_DIM_RATE, base.y * PANEL_DEAD_DIM_RATE, base.z * PANEL_DEAD_DIM_RATE);
			}
			else {
				const auto it = _panelDimRestore.find(node);
				if (it != _panelDimRestore.end()) {
					node->modulate = it->second;
					_panelDimRestore.erase(it);
				}
			}
		}
	}
}