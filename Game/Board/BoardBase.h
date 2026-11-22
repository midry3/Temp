#pragma once
#include <Scene.h>
#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <vector>
#include "BoardTile.h"
#include "BoardUnit.h"

class Sprite2D;

namespace Game {
	class BattleCharacter;

	struct BoardEnemyAction {
		SharedPtr<BoardUnit> enemy;
		int targetX = 0;
		int targetY = 0;
	};

	class BoardBase : public Scene {
	public:
		using EnemyActionDecider = std::function<std::optional<BoardEnemyAction>(BoardBase&)>;

	private:
		int _width = 0;
		int _height = 0;
		Vector2D _tileSize;
		Vector2D _boardPosition;
		std::vector<SharedPtr<BoardTile>> _tiles;
		std::vector<SharedPtr<BoardUnit>> _units;
		std::vector<SharedPtr<Sprite2D>> _objects;
		SharedPtr<BoardUnit> _selectedUnit;
		int _turn = 1;
		bool _isEnemyTurn = false;
		bool _isAnimating = false;
		std::optional<BoardEnemyAction> _nextEnemyAction;
		EnemyActionDecider _customEnemyActionDecider;

		void UpdateLayout();
		void BuildTiles(const std::map<BoardTerrain, std::string>& textures, const std::vector<BoardTileData>& tiles);
		void BuildObjects(const std::vector<BoardObjectData>& objects);
		void BuildUnits(const std::vector<BoardUnitData>& units);
		void SelectUnit(const SharedPtr<BoardUnit>& unit);
		void SelectTile(int x, int y);
		void CancelSelection();
		void UpdateMoveHints();
		void ExecuteEnemyTurn();
		void ClearEnemyIntentVisuals();

	protected:
		void _Load() override;
		bool LoadBoard(const std::string& file);

	public:
		void EndTurn();
		void UpdateEnemyIntentVisuals();
		void SetEnemyActionDecider(const EnemyActionDecider& decider) { _customEnemyActionDecider = decider; }

		[[nodiscard]] virtual std::optional<BoardEnemyAction> DecideEnemyAction();

		bool MoveUnit(const SharedPtr<BoardUnit>& unit, int x, int y, bool allowOccupiedByTarget = false, const std::function<void()>& onComplete = nullptr);
		[[nodiscard]] bool IsMoveAllowed(const BoardUnit& unit, int x, int y, bool allowOccupiedByTarget = false) const;
		[[nodiscard]] bool CanMoveTo(const BoardUnit& unit, int x, int y, bool allowOccupiedByTarget = false) const { return IsMoveAllowed(unit, x, y, allowOccupiedByTarget); }
		[[nodiscard]] bool IsPathClear(int startX, int startY, int endX, int endY) const;
		[[nodiscard]] bool IsInside(int x, int y) const;
		[[nodiscard]] bool IsOccupied(int x, int y, const BoardUnit* except = nullptr) const;
		[[nodiscard]] int GetTileIndex(int x, int y) const;
		[[nodiscard]] Vector2D GetTilePosition(int x, int y) const;
		[[nodiscard]] SharedPtr<BoardTile> GetTile(int x, int y) const;

		[[nodiscard]] std::vector<SharedPtr<BoardUnit>> GetUnitsByTeam(BoardTeam team) const;
		[[nodiscard]] std::vector<SharedPtr<BoardUnit>> GetAllyUnits() const { return GetUnitsByTeam(BoardTeam::Ally); }
		[[nodiscard]] std::vector<SharedPtr<BoardUnit>> GetEnemyUnits() const { return GetUnitsByTeam(BoardTeam::Enemy); }
		[[nodiscard]] SharedPtr<BoardUnit> GetUnitAt(int x, int y) const;
		[[nodiscard]] std::vector<BoardOffset> GetEnemyMoveOffsets(const BoardUnit& unit) const;
		[[nodiscard]] std::vector<std::pair<int, int>> GetReachablePositions(const BoardUnit& unit, bool allowOccupiedByTarget = false) const;
		[[nodiscard]] int GetManhattanDistance(int x1, int y1, int x2, int y2) const;
		[[nodiscard]] SharedPtr<BoardUnit> FindNearestAlly(int x, int y) const;
		[[nodiscard]] SharedPtr<BoardUnit> FindNearestEnemy(int x, int y) const;

		[[nodiscard]] SharedPtr<BattleCharacter> CreateBattleCharacterForUnit(const BoardUnit& unit) const;
		[[nodiscard]] int GetTurn() const { return _turn; }
		[[nodiscard]] bool IsEnemyTurn() const { return _isEnemyTurn; }
		[[nodiscard]] bool IsAnimating() const { return _isAnimating; }
		[[nodiscard]] const std::optional<BoardEnemyAction>& GetNextEnemyAction() const { return _nextEnemyAction; }
	};
}
