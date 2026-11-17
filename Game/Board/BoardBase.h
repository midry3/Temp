#pragma once
#include <Scene.h>
#include <map>
#include <set>
#include <vector>
#include "BoardTile.h"
#include "BoardUnit.h"

namespace Game {
	class BoardBase : public Scene {
	private:
		int _width = 0;
		int _height = 0;
		Vector2D _tileSize;
		Vector2D _boardPosition;
		std::vector<SharedPtr<BoardTile>> _tiles;
		std::vector<SharedPtr<BoardUnit>> _units;
		std::set<std::string> _movedAllies;
		SharedPtr<BoardUnit> _selectedUnit;
		int _turn = 1;

		bool LoadBoard(const std::string& file);
		void BuildTiles(const std::map<BoardTerrain, std::string>& textures, const std::vector<std::string>& terrainNames);
		void BuildUnits(const std::vector<BoardUnitData>& units);
		void SelectUnit(const SharedPtr<BoardUnit>& unit);
		void SelectTile(int x, int y);
		void CancelSelection();
		void UpdateMoveHints();
		void EndTurnIfAlliesMoved();
		void MoveEnemies();
		bool MoveUnit(const SharedPtr<BoardUnit>& unit, int x, int y);
		bool IsMoveAllowed(const BoardUnit& unit, int x, int y) const;
		bool IsPathClear(int startX, int startY, int endX, int endY) const;
		bool IsInside(int x, int y) const;
		bool IsOccupied(int x, int y, const BoardUnit* except = nullptr) const;
		int GetTileIndex(int x, int y) const;
		Vector2D GetTilePosition(int x, int y) const;
		SharedPtr<BoardTile> GetTile(int x, int y) const;

	protected:
		void _Load() override;

	public:
		void EndTurn();
		[[nodiscard]] int GetTurn() const { return _turn; }
	};
}
