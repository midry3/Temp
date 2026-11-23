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
	class BattleBase;
	class BoardWinScreen;
	class CoopSelectModal;

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
		bool _isCoopSelecting = false;
		std::optional<BoardEnemyAction> _nextEnemyAction;
		EnemyActionDecider _customEnemyActionDecider;
		std::function<void()> _pendingTransition;
		SharedPtr<CoopSelectModal> _coopModal;
		SharedPtr<BoardUnit> _battleEnemyUnit;
		SharedPtr<BoardUnit> _battleWarningEnemy;
		bool _battleStartedInEnemyTurn = false;
		SharedPtr<BoardWinScreen> _winScreen;

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
		void OnReturnedFromBattle();
		void ShowWinScreen();

	protected:
		void _Load() override;
		void _Update(float dt) override;
		bool LoadBoard(const std::string& file);

	public:
		void EndTurn();
		void UpdateEnemyIntentVisuals();
		void SetEnemyActionDecider(const EnemyActionDecider& decider) { _customEnemyActionDecider = decider; }

		[[nodiscard]] virtual std::optional<BoardEnemyAction> DecideEnemyAction();

		[[nodiscard]] bool CanReachTile(const BoardUnit& unit, int targetX, int targetY) const;
		virtual void OpenCoopSelect(const SharedPtr<BoardUnit>& primaryAlly, const SharedPtr<BoardUnit>& enemy);
		virtual void SetupBattle(BattleBase& battle, const SharedPtr<BoardUnit>& allyUnit, const SharedPtr<BoardUnit>& enemyUnit, const std::vector<SharedPtr<BoardUnit>>& coopAllies = {});
		virtual void StartBattle(const SharedPtr<BoardUnit>& allyUnit, const SharedPtr<BoardUnit>& enemyUnit, const std::vector<SharedPtr<BoardUnit>>& coopAllies = {});
		virtual void TransitionToBattle(const SharedPtr<BattleBase>& battleScene);

		// 敵ユニットを盤面から消去する。実際の消去は PlayEnemyRemoveEffect の完了後に行われる。
		void RemoveEnemyUnit(const SharedPtr<BoardUnit>& enemy, const std::function<void()>& onComplete = nullptr);
		// 敵消去時の演出。既定では演出なし（即座に onComplete を呼ぶ）。
		// 演出を挟みたい場合は派生クラスでオーバーライドし、演出の完了時に onComplete を呼ぶこと。
		virtual void PlayEnemyRemoveEffect(const SharedPtr<BoardUnit>& enemy, const std::function<void()>& onComplete);

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
		[[nodiscard]] bool IsCoopSelecting() const { return _isCoopSelecting; }
		[[nodiscard]] const std::optional<BoardEnemyAction>& GetNextEnemyAction() const { return _nextEnemyAction; }
	};
}
