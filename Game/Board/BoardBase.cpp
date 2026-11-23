#include "BoardBase.h"
#include "CoopSelectModal.h"
#include "../Battle/BattleBase.h"
#include "../Battle/BattleCharacter/BattleCharacterFactory.h"
#include "../Battle/BattleCharacter/Enemy/EnemyBattleCharacter.h"
#include <Animation/Tween.h>
#include <Asset/AssetManager.h>
#include <Scene/Texture/Sprite2D.h>
#include <Game/Data/CommonData.h>
#include <SceneManager.h>
#include <lua.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace Game {
	namespace {
		std::string ReadString(lua_State* state, int table, const char* key, const std::string& fallback = "") {
			lua_getfield(state, table, key);
			const char* value = lua_tostring(state, -1);
			const std::string result = value ? value : fallback;
			lua_pop(state, 1);
			return result;
		}

		int ReadInteger(lua_State* state, int table, const char* key, int fallback = 0) {
			lua_getfield(state, table, key);
			const int result = lua_isnumber(state, -1) ? static_cast<int>(lua_tointeger(state, -1)) : fallback;
			lua_pop(state, 1);
			return result;
		}

		double ReadNumber(lua_State* state, int table, const char* key, double fallback = 0.0) {
			lua_getfield(state, table, key);
			const double result = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : fallback;
			lua_pop(state, 1);
			return result;
		}

		bool ReadBool(lua_State* state, int table, const char* key, bool fallback = false) {
			lua_getfield(state, table, key);
			const bool result = lua_isboolean(state, -1) ? lua_toboolean(state, -1) != 0 : fallback;
			lua_pop(state, 1);
			return result;
		}

		void ReadPosition(lua_State* state, int table, double& x, double& y) {
			lua_getfield(state, table, "position");
			if (lua_istable(state, -1)) {
				x = ReadNumber(state, -1, "x", x);
				y = ReadNumber(state, -1, "y", y);
			}
			lua_pop(state, 1);
		}

		BoardTerrain ParseTerrain(const std::string& value) {
			if (value == "disabled") return BoardTerrain::Disabled;
			if (value == "other") return BoardTerrain::Other;
			return BoardTerrain::Normal;
		}

		BoardTeam ParseTeam(const std::string& value) {
			return value == "enemy" ? BoardTeam::Enemy : BoardTeam::Ally;
		}

		BoardMoveType ParseMoveType(const std::string& value) {
			if (value == "diagonal_unlimited") return BoardMoveType::DiagonalUnlimited;
			if (value == "range_two") return BoardMoveType::RangeTwo;
			if (value == "orthogonal_unlimited") return BoardMoveType::OrthogonalUnlimited;
			return BoardMoveType::Adjacent;
		}
	}

	void BoardBase::_Load() {
		CommonData::CurrentBoard = std::dynamic_pointer_cast<BoardBase>(shared_from_this());
	}

	void BoardBase::_Update(float dt) {
		Scene::_Update(dt);
		if (_pendingTransition) {
			auto transition = std::move(_pendingTransition);
			_pendingTransition = nullptr;
			transition();
			return;
		}
	}

	bool BoardBase::LoadBoard(const std::string& file) {
		lua_State* state = luaL_newstate();
		if (not state) return false;
		luaL_openlibs(state);
		const auto resolvedFile = AssetManager.ResolvePath(file);
		if (luaL_dofile(state, resolvedFile.string().c_str()) != LUA_OK) {
			std::cerr << "Board Lua load error (" << file << "): "
				<< lua_tostring(state, -1) << std::endl;
			lua_close(state);
			return false;
		}

		lua_getglobal(state, "board");
		if (not lua_istable(state, -1)) {
			lua_pop(state, 1);
			lua_close(state);
			return false;
		}

		_width = ReadInteger(state, -1, "width");
		_height = ReadInteger(state, -1, "height");
		_tileSize.x = ReadInteger(state, -1, "tileSize", 64);
		_tileSize.y = _tileSize.x;
		double unusedX = 0.0;
		double unusedY = 0.0;
		ReadPosition(state, -1, unusedX, unusedY);
		_boardPosition = Vector2D(0.0, 0.0);
		if (not (0 < _width) || not (0 < _height)) {
			lua_pop(state, 1);
			lua_close(state);
			return false;
		}

		std::map<BoardTerrain, std::string> textures;
		lua_getfield(state, -1, "textures");
		if (lua_istable(state, -1)) {
			textures[BoardTerrain::Normal] = ReadString(state, -1, "normal");
			textures[BoardTerrain::Disabled] = ReadString(state, -1, "disabled", textures[BoardTerrain::Normal]);
			textures[BoardTerrain::Other] = ReadString(state, -1, "other", textures[BoardTerrain::Normal]);
		}
		lua_pop(state, 1);

		std::vector<BoardTileData> tiles;
		lua_getfield(state, -1, "tiles");
		if (lua_istable(state, -1)) {
			const int count = static_cast<int>(lua_rawlen(state, -1));
			for (int index = 1; index <= count; index++) {
				lua_rawgeti(state, -1, index);
				if (lua_isstring(state, -1)) {
					const auto terrain = ParseTerrain(lua_tostring(state, -1));
					const auto texture = textures.find(terrain);
					tiles.push_back({ terrain, texture == textures.end() ? "" : texture->second });
				}
				else if (lua_istable(state, -1)) {
					const auto terrainName = ReadString(state, -1, "terrain", "normal");
					auto terrain = ParseTerrain(terrainName);
					const bool passable = ReadBool(state, -1, "passable", terrain != BoardTerrain::Disabled);
					if (not passable) terrain = BoardTerrain::Disabled;
					else if (terrain == BoardTerrain::Disabled) terrain = BoardTerrain::Normal;
					const auto explicitTexture = ReadString(state, -1, "texture");
					const auto texture = textures.find(terrain);
					tiles.push_back({ terrain, explicitTexture.empty()
						? (texture == textures.end() ? "" : texture->second) : explicitTexture });
				}
				else {
					const auto texture = textures.find(BoardTerrain::Normal);
					tiles.push_back({ BoardTerrain::Normal, texture == textures.end() ? "" : texture->second });
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);

		std::vector<BoardObjectData> objects;
		lua_getfield(state, -1, "objects");
		if (lua_istable(state, -1)) {
			const int count = static_cast<int>(lua_rawlen(state, -1));
			for (int index = 1; index <= count; ++index) {
				lua_rawgeti(state, -1, index);
				if (lua_istable(state, -1)) {
					const double scaleVal = ReadNumber(state, -1, "scale", 1.0);
					objects.push_back({
						ReadString(state, -1, "texture"),
						ReadInteger(state, -1, "x"),
						ReadInteger(state, -1, "y"),
						ReadInteger(state, -1, "width", 1),
						ReadInteger(state, -1, "height", 1),
						ReadNumber(state, -1, "scaleX", scaleVal),
						ReadNumber(state, -1, "scaleY", scaleVal)
					});
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);

		std::vector<BoardUnitData> units;
		lua_getfield(state, -1, "units");
		if (lua_istable(state, -1)) {
			const int count = static_cast<int>(lua_rawlen(state, -1));
			for (int index = 1; index <= count; index++) {
				lua_rawgeti(state, -1, index);
				if (lua_istable(state, -1)) {
					BoardUnitData unit;
					unit.id = ReadString(state, -1, "id");
					unit.displayName = ReadString(state, -1, "displayName");
					unit.characterType = ParseBoardCharacterType(ReadString(state, -1, "character", "plea"));
					unit.team = ParseTeam(ReadString(state, -1, "team", "ally"));
					unit.moveType = ParseMoveType(ReadString(state, -1, "move", "adjacent"));
					unit.texture = ReadString(state, -1, "icon", ReadString(state, -1, "texture"));
					unit.x = ReadInteger(state, -1, "x");
					unit.y = ReadInteger(state, -1, "y");
					units.push_back(unit);
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);
		lua_pop(state, 1);
		lua_close(state);

		UpdateLayout();
		BuildTiles(textures, tiles);
		BuildObjects(objects);
		BuildUnits(units);
		UpdateEnemyIntentVisuals();
		return true;
	}

	void BoardBase::UpdateLayout() {
		const Vector2D size(static_cast<double>(_width) * _tileSize.x, static_cast<double>(_height) * _tileSize.y);
		Vector2D window = PropertyManager.GetSettingWindowSize();
		if (not (0.0 < window.x) || not (0.0 < window.y)) {
			window = PropertyManager.GetRealWindowSize();
		}
		if (0.0 < size.x && 0.0 < size.y && 0.0 < window.x && 0.0 < window.y) {
			const double scaleFactor = Min(window.x / size.x, window.y / size.y);
			SetScale(scaleFactor);
			const Vector2D offset = (window - size * scaleFactor) * 0.5;
			SetPosition(offset);
		}
	}

	void BoardBase::BuildTiles(const std::map<BoardTerrain, std::string>& textures, const std::vector<BoardTileData>& tiles) {
		const auto defaultIt = textures.find(BoardTerrain::Normal);
		const auto defaultTexture = defaultIt == textures.end() ? std::string() : defaultIt->second;
		for (int y = 0; y < _height; y++) {
			for (int x = 0; x < _width; x++) {
				const auto index = GetTileIndex(x, y);
				const auto tileData = index < static_cast<int>(tiles.size())
					? tiles[index] : BoardTileData{};
				const auto terrain = tileData.terrain;
				const auto texturePath = tileData.texture.empty() ? defaultTexture : tileData.texture;
				const auto& tile = CreateScene<BoardTile>();
				tile->SetTile(terrain, texturePath.empty() ? nullptr : LoadAsset<Texture>(texturePath), _tileSize);
				tile->SetPosition(GetTilePosition(x, y));
				_tiles.push_back(tile);
				tile->LeftPressed += [this, x, y](const MouseEvent& event) {
					if (event.just) SelectTile(x, y);
					};
				tile->RightPressed += [this](const MouseEvent& event) {
					if (event.just) CancelSelection();
					};
			}
		}
	}

	void BoardBase::BuildObjects(const std::vector<BoardObjectData>& objects) {
		for (const auto& data : objects) {
			if (data.texture.empty()) continue;
			const auto& sprite = CreateScene<Sprite2D>();
			sprite->SetTexture(LoadAsset<Texture>(data.texture));
			sprite->SetScale({ data.scaleX, data.scaleY });
			sprite->SetPosition(GetTilePosition(data.x, data.y));
			_objects.push_back(sprite);
		}
	}

	void BoardBase::BuildUnits(const std::vector<BoardUnitData>& units) {
		for (const auto& data : units) {
			const auto& unit = CreateScene<BoardUnit>();
			unit->SetUnit(data, data.texture.empty() ? nullptr : LoadAsset<Texture>(data.texture), _tileSize);
			unit->SetPosition(GetTilePosition(data.x, data.y));
			if (not unit->IsAlly()) {
				const auto battleChara = CreateBattleCharacterForUnit(*unit);
				if (const auto enemyChara = std::dynamic_pointer_cast<EnemyBattleCharacter>(battleChara)) {
					unit->SetRelativeMoveOffsets(enemyChara->GetRelativeMoveOffsets());
				}
			}
			unit->LeftPressed += [this, unit](const MouseEvent& event) {
				if (event.just) SelectUnit(unit);
				};
			unit->RightPressed += [this](const MouseEvent& event) {
				if (event.just) CancelSelection();
				};
			_units.push_back(unit);
		}
	}

	void BoardBase::SelectUnit(const SharedPtr<BoardUnit>& unit) {
		if (_isEnemyTurn || _isAnimating || _isCoopSelecting) return;
		if (not unit->IsAlly()) {
			if (_selectedUnit) {
				SelectTile(unit->GetData().x, unit->GetData().y);
			}
			return;
		}
		CancelSelection();
		_selectedUnit = unit;
		GetTile(unit->GetData().x, unit->GetData().y)->SetSelected(true);
		UpdateMoveHints();
	}

	void BoardBase::SelectTile(int x, int y) {
		if (_isEnemyTurn || _isAnimating || _isCoopSelecting) return;
		if (not _selectedUnit) return;
		const auto movingUnit = _selectedUnit;
		if (IsMoveAllowed(*movingUnit, x, y, true)) {
			const auto targetUnit = GetUnitAt(x, y);
			const bool isEnemyEncounter = targetUnit && not targetUnit->IsAlly();
			CancelSelection();
			MoveUnit(movingUnit, x, y, true, [this, movingUnit, targetUnit, isEnemyEncounter]() {
				if (isEnemyEncounter) {
					OpenCoopSelect(movingUnit, targetUnit);
				}
				else {
					ExecuteEnemyTurn();
				}
			});
		}
	}

	void BoardBase::CancelSelection() {
		for (const auto& tile : _tiles) {
			tile->SetSelected(false);
			tile->SetMoveHint(false);
		}
		_selectedUnit.reset();
	}

	void BoardBase::UpdateMoveHints() {
		if (not _selectedUnit) return;
		for (int y = 0; y < _height; y++) {
			for (int x = 0; x < _width; x++) {
				GetTile(x, y)->SetMoveHint(IsMoveAllowed(*_selectedUnit, x, y, true));
			}
		}
	}

	void BoardBase::ClearEnemyIntentVisuals() {
		for (const auto& enemy : GetEnemyUnits()) {
			enemy->SetIntentHint(false);
		}
		for (const auto& tile : _tiles) {
			tile->SetEnemyTargetHint(false);
		}
	}

	void BoardBase::UpdateEnemyIntentVisuals() {
		ClearEnemyIntentVisuals();
		_nextEnemyAction.reset();
		if (_isEnemyTurn) return;

		_nextEnemyAction = DecideEnemyAction();
		if (_nextEnemyAction.has_value()) {
			if (_nextEnemyAction->enemy) {
				_nextEnemyAction->enemy->SetIntentHint(true);
			}
			const auto targetTile = GetTile(_nextEnemyAction->targetX, _nextEnemyAction->targetY);
			if (targetTile) {
				targetTile->SetEnemyTargetHint(true);
			}
		}
	}

	void BoardBase::ExecuteEnemyTurn() {
		_isEnemyTurn = true;
		ClearEnemyIntentVisuals();

		std::optional<BoardEnemyAction> action = _nextEnemyAction;
		_nextEnemyAction.reset();

		if (not action.has_value() || not action->enemy) {
			action = DecideEnemyAction();
		}

		if (action.has_value() && action->enemy) {
			const auto targetUnit = GetUnitAt(action->targetX, action->targetY);
			const bool isAllyEncounter = targetUnit && targetUnit->IsAlly();
			if (CanMoveTo(*action->enemy, action->targetX, action->targetY, true)) {
				MoveUnit(action->enemy, action->targetX, action->targetY, true, [this, enemy = action->enemy, targetUnit, isAllyEncounter]() {
					if (isAllyEncounter) {
						OpenCoopSelect(targetUnit, enemy);
					}
					else {
						_turn++;
						_isEnemyTurn = false;
						UpdateEnemyIntentVisuals();
					}
				});
				return;
			}
			const auto fallbackAction = DecideEnemyAction();
			if (fallbackAction.has_value() && fallbackAction->enemy && CanMoveTo(*fallbackAction->enemy, fallbackAction->targetX, fallbackAction->targetY, true)) {
				const auto fallbackTarget = GetUnitAt(fallbackAction->targetX, fallbackAction->targetY);
				const bool fallbackEncounter = fallbackTarget && fallbackTarget->IsAlly();
				MoveUnit(fallbackAction->enemy, fallbackAction->targetX, fallbackAction->targetY, true, [this, enemy = fallbackAction->enemy, fallbackTarget, fallbackEncounter]() {
					if (fallbackEncounter) {
						OpenCoopSelect(fallbackTarget, enemy);
					}
					else {
						_turn++;
						_isEnemyTurn = false;
						UpdateEnemyIntentVisuals();
					}
				});
				return;
			}
		}

		_turn++;
		_isEnemyTurn = false;
		UpdateEnemyIntentVisuals();
	}

	void BoardBase::EndTurn() {
		if (_isEnemyTurn || _isAnimating || _isCoopSelecting) return;
		CancelSelection();
		ExecuteEnemyTurn();
	}

	bool BoardBase::CanReachTile(const BoardUnit& unit, int targetX, int targetY) const {
		if (not IsInside(targetX, targetY)) return false;
		if (not GetTile(targetX, targetY)->IsPassable()) return false;
		const auto& data = unit.GetData();
		const auto dx = targetX - data.x;
		const auto dy = targetY - data.y;
		if (dx == 0 && dy == 0) return false;

		const auto absX = std::abs(dx);
		const auto absY = std::abs(dy);
		if (data.moveType == BoardMoveType::Adjacent) return not (1 < absX) && not (1 < absY);
		if (data.moveType == BoardMoveType::RangeTwo) return not (2 < absX) && not (2 < absY);
		if (data.moveType == BoardMoveType::DiagonalUnlimited) {
			if (dx == 0 || dy == 0) return not (1 < absX) && not (1 < absY);
			if (absX != absY) return false;
			return IsPathClear(data.x, data.y, targetX, targetY);
		}
		if (dx != 0 && dy != 0) return false;
		return IsPathClear(data.x, data.y, targetX, targetY);
	}

	void BoardBase::OpenCoopSelect(const SharedPtr<BoardUnit>& primaryAlly, const SharedPtr<BoardUnit>& enemy) {
		_isCoopSelecting = true;
		std::vector<SharedPtr<BoardUnit>> availableAllies;
		const int battleX = enemy->GetData().x;
		const int battleY = enemy->GetData().y;
		for (const auto& unit : GetAllyUnits()) {
			if (unit != primaryAlly && CanReachTile(*unit, battleX, battleY)) {
				availableAllies.push_back(unit);
			}
		}

		_coopModal = CreateScene<CoopSelectModal>();
		_coopModal->Setup(primaryAlly, enemy, availableAllies, _tileSize, [this, primaryAlly, enemy](const std::vector<SharedPtr<BoardUnit>>& selectedCoopAllies) {
			if (_coopModal) {
				_coopModal->Free();
				_coopModal = nullptr;
			}
			_isCoopSelecting = false;
			_pendingTransition = [this, primaryAlly, enemy, selectedCoopAllies]() {
				StartBattle(primaryAlly, enemy, selectedCoopAllies);
			};
		});
	}

	void BoardBase::SetupBattle(BattleBase& battle, const SharedPtr<BoardUnit>& allyUnit, const SharedPtr<BoardUnit>& enemyUnit, const std::vector<SharedPtr<BoardUnit>>& coopAllies) {
		if (allyUnit) {
			const auto allyChar = CreateBattleCharacterForUnit(*allyUnit);
			if (allyChar) {
				battle.AddAlly(allyChar);
			}
		}
		for (const auto& coopUnit : coopAllies) {
			if (coopUnit && coopUnit != allyUnit) {
				const auto coopChar = CreateBattleCharacterForUnit(*coopUnit);
				if (coopChar) {
					battle.AddAlly(coopChar);
				}
			}
		}
		if (enemyUnit) {
			const auto enemyChar = CreateBattleCharacterForUnit(*enemyUnit);
			if (enemyChar) {
				battle.AddEnemy(enemyChar);
			}
		}
	}

	void BoardBase::StartBattle(const SharedPtr<BoardUnit>& allyUnit, const SharedPtr<BoardUnit>& enemyUnit, const std::vector<SharedPtr<BoardUnit>>& coopAllies) {
		const auto battleScene = MakeShared<BattleBase>();
		SetupBattle(*battleScene, allyUnit, enemyUnit, coopAllies);
		TransitionToBattle(battleScene);
	}

	void BoardBase::TransitionToBattle(const SharedPtr<BattleBase>& battleScene) {
		SceneManager.TransSceneTo(battleScene, 1.0, 0.3, false);
	}

	bool BoardBase::MoveUnit(const SharedPtr<BoardUnit>& unit, int x, int y, bool allowOccupiedByTarget, const std::function<void()>& onComplete) {
		if (not IsMoveAllowed(*unit, x, y, allowOccupiedByTarget)) return false;
		_isAnimating = true;
		unit->SetBoardPosition(x, y);
		const auto& targetPos = GetTilePosition(x, y);
		const auto n = unit->position.Perpendicular().Normalized();
		const auto midPos = unit->position + (targetPos - unit->position) * 0.7 - Sign(n.y) * n * 50.0;
		const auto& tg = unit->CreateTweenGroup();
		tg->CreateTween(&unit->position, midPos, 0.35, Easing::EaseOutCubic);
		tg->CreateTween(&unit->position, targetPos, 0.35, Easing::EaseInCubic);

		const auto& tg2 = unit->CreateTweenGroup();
		tg2->CreateTween(&unit->scale, Vec2{ 0.8, 1.2 }, 0.35, Easing::EaseOutCubic);
		tg2->CreateTween(&unit->scale, Vec2{ 1.2, 0.8 }, 0.35, Easing::EaseOutCubic);
		tg2->CreateTween(&unit->scale, Vec2D_1, 0.15);

		tg->OnComplete([this, onComplete]() {
			_isAnimating = false;
			if (onComplete) onComplete();
		});
		return true;
	}

	std::vector<SharedPtr<BoardUnit>> BoardBase::GetUnitsByTeam(BoardTeam team) const {
		std::vector<SharedPtr<BoardUnit>> result;
		for (const auto& unit : _units) {
			if (unit->GetData().team == team) {
				result.push_back(unit);
			}
		}
		return result;
	}

	SharedPtr<BoardUnit> BoardBase::GetUnitAt(int x, int y) const {
		for (const auto& unit : _units) {
			if (unit->GetData().x == x && unit->GetData().y == y) {
				return unit;
			}
		}
		return nullptr;
	}

	std::vector<BoardOffset> BoardBase::GetEnemyMoveOffsets(const BoardUnit& unit) const {
		const auto& offsets = unit.GetRelativeMoveOffsets();
		if (not offsets.empty()) return offsets;

		const auto battleChara = CreateBattleCharacterForUnit(unit);
		if (const auto enemyChara = std::dynamic_pointer_cast<EnemyBattleCharacter>(battleChara)) {
			const auto enemyOffsets = enemyChara->GetRelativeMoveOffsets();
			if (not enemyOffsets.empty()) return enemyOffsets;
		}

		return {
			{ -1, -1 }, { 0, -1 }, { 1, -1 },
			{ -1,  0 },            { 1,  0 },
			{ -1,  1 }, { 0,  1 }, { 1,  1 }
		};
	}

	int BoardBase::GetManhattanDistance(int x1, int y1, int x2, int y2) const {
		return std::abs(x1 - x2) + std::abs(y1 - y2);
	}

	SharedPtr<BoardUnit> BoardBase::FindNearestAlly(int x, int y) const {
		SharedPtr<BoardUnit> nearest;
		int minDistance = 999999;
		for (const auto& ally : GetAllyUnits()) {
			const int dist = GetManhattanDistance(x, y, ally->GetData().x, ally->GetData().y);
			if (dist < minDistance) {
				minDistance = dist;
				nearest = ally;
			}
		}
		return nearest;
	}

	SharedPtr<BoardUnit> BoardBase::FindNearestEnemy(int x, int y) const {
		SharedPtr<BoardUnit> nearest;
		int minDistance = 999999;
		for (const auto& enemy : GetEnemyUnits()) {
			const int dist = GetManhattanDistance(x, y, enemy->GetData().x, enemy->GetData().y);
			if (dist < minDistance) {
				minDistance = dist;
				nearest = enemy;
			}
		}
		return nearest;
	}

	std::vector<std::pair<int, int>> BoardBase::GetReachablePositions(const BoardUnit& unit, bool allowOccupiedByTarget) const {
		std::vector<std::pair<int, int>> positions;
		if (not unit.IsAlly()) {
			const auto offsets = GetEnemyMoveOffsets(unit);
			for (const auto& offset : offsets) {
				const int targetX = unit.GetData().x + offset.dx;
				const int targetY = unit.GetData().y + offset.dy;
				if (CanMoveTo(unit, targetX, targetY, allowOccupiedByTarget)) {
					positions.push_back({ targetX, targetY });
				}
			}
			return positions;
		}

		for (int y = 0; y < _height; y++) {
			for (int x = 0; x < _width; x++) {
				if (CanMoveTo(unit, x, y, allowOccupiedByTarget)) {
					positions.push_back({ x, y });
				}
			}
		}
		return positions;
	}

	std::optional<BoardEnemyAction> BoardBase::DecideEnemyAction() {
		if (_customEnemyActionDecider) {
			return _customEnemyActionDecider(*this);
		}
		const auto enemies = GetEnemyUnits();
		if (enemies.empty()) return std::nullopt;

		const auto allies = GetAllyUnits();
		if (allies.empty()) return std::nullopt;

		// 優先度1: 移動可能マスにプレイヤーがいればそのマスに移動する(予定)
		for (const auto& enemy : enemies) {
			const auto reachable = GetReachablePositions(*enemy, true);
			for (const auto& [tx, ty] : reachable) {
				const auto occupant = GetUnitAt(tx, ty);
				if (occupant && occupant->IsAlly()) {
					return BoardEnemyAction{ enemy, tx, ty };
				}
			}
		}

		// 優先度2: 最も近いプレイヤーに近づくように移動する
		SharedPtr<BoardUnit> bestEnemy;
		int bestDist = 999999;
		std::pair<int, int> bestTarget = { -1, -1 };

		for (const auto& enemy : enemies) {
			const auto nearestAlly = FindNearestAlly(enemy->GetData().x, enemy->GetData().y);
			if (not nearestAlly) continue;

			const int currentDist = GetManhattanDistance(
				enemy->GetData().x, enemy->GetData().y,
				nearestAlly->GetData().x, nearestAlly->GetData().y
			);

			const auto reachable = GetReachablePositions(*enemy, false);
			for (const auto& [tx, ty] : reachable) {
				const int dist = GetManhattanDistance(
					tx, ty,
					nearestAlly->GetData().x, nearestAlly->GetData().y
				);
				if (dist < currentDist && dist < bestDist) {
					bestDist = dist;
					bestEnemy = enemy;
					bestTarget = { tx, ty };
				}
			}
		}

		if (bestEnemy && not (bestTarget.first < 0)) {
			return BoardEnemyAction{ bestEnemy, bestTarget.first, bestTarget.second };
		}

		// 優先度3: 近づけない場合でも、移動可能マスがあれば最初の敵を動かす
		for (const auto& enemy : enemies) {
			const auto reachable = GetReachablePositions(*enemy, false);
			if (not reachable.empty()) {
				return BoardEnemyAction{ enemy, reachable.front().first, reachable.front().second };
			}
		}

		return std::nullopt;
	}

	bool BoardBase::IsMoveAllowed(const BoardUnit& unit, int x, int y, bool allowOccupiedByTarget) const {
		if (not IsInside(x, y)) return false;
		if (not GetTile(x, y)->IsPassable()) return false;

		const auto occupant = GetUnitAt(x, y);
		if (occupant && occupant.get() != &unit) {
			if (allowOccupiedByTarget) {
				if (unit.IsAlly() == occupant->IsAlly()) {
					return false;
				}
			}
			else {
				return false;
			}
		}

		const auto& data = unit.GetData();
		const auto dx = x - data.x;
		const auto dy = y - data.y;
		if (dx == 0 && dy == 0) return false;

		// 敵の場合は相対移動マス定義を判定
		if (not unit.IsAlly()) {
			const auto offsets = GetEnemyMoveOffsets(unit);
			for (const auto& offset : offsets) {
				if (offset.dx == dx && offset.dy == dy) {
					return true;
				}
			}
			return false;
		}

		// 味方の場合は既存の MoveType 判定
		const auto absX = std::abs(dx);
		const auto absY = std::abs(dy);
		if (data.moveType == BoardMoveType::Adjacent) return not (1 < absX) && not (1 < absY);
		if (data.moveType == BoardMoveType::RangeTwo) return not (2 < absX) && not (2 < absY);
		if (data.moveType == BoardMoveType::DiagonalUnlimited) {
			if (dx == 0 || dy == 0) return not (1 < absX) && not (1 < absY);
			if (absX != absY) return false;
			return IsPathClear(data.x, data.y, x, y);
		}
		if (dx != 0 && dy != 0) return false;
		return IsPathClear(data.x, data.y, x, y);
	}

	bool BoardBase::IsPathClear(int startX, int startY, int endX, int endY) const {
		const auto stepX = endX == startX ? 0 : (endX < startX ? -1 : 1);
		const auto stepY = endY == startY ? 0 : (endY < startY ? -1 : 1);
		auto x = startX + stepX;
		auto y = startY + stepY;
		while (x != endX || y != endY) {
			if (not GetTile(x, y)->IsPassable() || IsOccupied(x, y)) return false;
			x += stepX;
			y += stepY;
		}
		return true;
	}

	bool BoardBase::IsInside(int x, int y) const {
		return not (x < 0) && x < _width && not (y < 0) && y < _height;
	}

	bool BoardBase::IsOccupied(int x, int y, const BoardUnit* except) const {
		for (const auto& unit : _units) {
			if (unit.get() != except && unit->GetData().x == x && unit->GetData().y == y) return true;
		}
		return false;
	}

	int BoardBase::GetTileIndex(int x, int y) const {
		return y * _width + x;
	}

	Vector2D BoardBase::GetTilePosition(int x, int y) const {
		return _boardPosition + Vector2D(x * _tileSize.x, y * _tileSize.y);
	}

	SharedPtr<BoardTile> BoardBase::GetTile(int x, int y) const {
		if (not IsInside(x, y)) return nullptr;
		return _tiles.at(GetTileIndex(x, y));
	}

	SharedPtr<BattleCharacter> BoardBase::CreateBattleCharacterForUnit(const BoardUnit& unit) const {
		return CreateBattleCharacter(unit.GetData().characterType);
	}
}
