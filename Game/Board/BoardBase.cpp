#include "BoardBase.h"
#include <Asset/AssetManager.h>
#include <lua.hpp>
#include <algorithm>
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

		void ReadPosition(lua_State* state, int table, double& x, double& y) {
			lua_getfield(state, table, "position");
			if (lua_istable(state, -1)) {
				x = static_cast<double>(ReadInteger(state, -1, "x", static_cast<int>(x)));
				y = static_cast<double>(ReadInteger(state, -1, "y", static_cast<int>(y)));
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
		LoadBoard("board/default.lua");
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
		ReadPosition(state, -1, _boardPosition.x, _boardPosition.y);
		if (_width <= 0 || _height <= 0) {
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

		std::vector<std::string> terrainNames;
		lua_getfield(state, -1, "tiles");
		if (lua_istable(state, -1)) {
			const int count = static_cast<int>(lua_rawlen(state, -1));
			for (int index = 1; index <= count; index++) {
				lua_rawgeti(state, -1, index);
				if (lua_isstring(state, -1)) {
					terrainNames.emplace_back(lua_tostring(state, -1));
				}
				else if (lua_istable(state, -1)) {
					terrainNames.push_back(ReadString(state, -1, "terrain", "normal"));
					const auto texture = ReadString(state, -1, "texture");
					if (not texture.empty()) textures[ParseTerrain(terrainNames.back())] = texture;
				}
				else {
					terrainNames.emplace_back("normal");
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
					unit.team = ParseTeam(ReadString(state, -1, "team", "ally"));
					unit.moveType = ParseMoveType(ReadString(state, -1, "move", "adjacent"));
					unit.texture = ReadString(state, -1, "texture");
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

		BuildTiles(textures, terrainNames);
		BuildUnits(units);
		return true;
	}

	void BoardBase::BuildTiles(const std::map<BoardTerrain, std::string>& textures, const std::vector<std::string>& terrainNames) {
		const auto defaultTexture = textures.at(BoardTerrain::Normal);
		for (int y = 0; y < _height; y++) {
			for (int x = 0; x < _width; x++) {
				const auto index = GetTileIndex(x, y);
				const auto terrainName = index < static_cast<int>(terrainNames.size()) ? terrainNames[index] : "normal";
				const auto terrain = ParseTerrain(terrainName);
				const auto textureIt = textures.find(terrain);
				const auto texturePath = textureIt == textures.end() ? defaultTexture : textureIt->second;
				const auto& tile = CreateScene<BoardTile>();
				tile->SetTile(terrain, LoadAsset<Texture>(texturePath), _tileSize);
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

	void BoardBase::BuildUnits(const std::vector<BoardUnitData>& units) {
		for (const auto& data : units) {
			const auto texturePath = data.texture.empty()
				? (data.team == BoardTeam::Ally ? "chara/plea/idle.png" : "chara/reko/idle.png")
				: data.texture;
			const auto& unit = CreateScene<BoardUnit>();
			unit->SetUnit(data, LoadAsset<Texture>(texturePath), _tileSize);
			unit->SetPosition(GetTilePosition(data.x, data.y));
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
		if (not unit->IsAlly()) return;
		if (_movedAllies.contains(unit->GetData().id)) return;
		CancelSelection();
		_selectedUnit = unit;
		GetTile(unit->GetData().x, unit->GetData().y)->SetSelected(true);
		UpdateMoveHints();
	}

	void BoardBase::SelectTile(int x, int y) {
		if (not _selectedUnit) return;
		if (MoveUnit(_selectedUnit, x, y)) {
			CancelSelection();
			EndTurnIfAlliesMoved();
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
				GetTile(x, y)->SetMoveHint(IsMoveAllowed(*_selectedUnit, x, y));
			}
		}
	}

	void BoardBase::EndTurnIfAlliesMoved() {
		for (const auto& unit : _units) {
			if (unit->IsAlly() && not _movedAllies.contains(unit->GetData().id)) return;
		}
		MoveEnemies();
		_movedAllies.clear();
		_turn++;
	}

	void BoardBase::MoveEnemies() {
		for (const auto& unit : _units) {
			if (not unit->IsAlly()) {
				const auto& data = unit->GetData();
				MoveUnit(unit, data.x, data.y + 1);
			}
		}
	}

	bool BoardBase::MoveUnit(const SharedPtr<BoardUnit>& unit, int x, int y) {
		if (not IsMoveAllowed(*unit, x, y)) return false;
		unit->SetBoardPosition(x, y);
		unit->SetPosition(GetTilePosition(x, y));
		if (unit->IsAlly()) _movedAllies.insert(unit->GetData().id);
		return true;
	}

	bool BoardBase::IsMoveAllowed(const BoardUnit& unit, int x, int y) const {
		if (not IsInside(x, y)) return false;
		if (not GetTile(x, y)->IsPassable()) return false;
		if (IsOccupied(x, y, &unit)) return false;
		const auto& data = unit.GetData();
		const auto dx = x - data.x;
		const auto dy = y - data.y;
		if (dx == 0 && dy == 0) return false;
		const auto absX = std::abs(dx);
		const auto absY = std::abs(dy);
		if (data.moveType == BoardMoveType::Adjacent) return absX <= 1 && absY <= 1;
		if (data.moveType == BoardMoveType::RangeTwo) return absX <= 2 && absY <= 2;
		if (data.moveType == BoardMoveType::DiagonalUnlimited) {
			if (dx == 0 || dy == 0) return absX <= 1 && absY <= 1;
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

	void BoardBase::EndTurn() {
		MoveEnemies();
		_movedAllies.clear();
		_turn++;
	}
}
