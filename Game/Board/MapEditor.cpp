#include "MapEditor.h"
#include "../Battle/BattleCharacter/BattleCharacterFactory.h"
#include <Asset/AssetManager.h>
#include <Camera.h>
#include <Mouse/Mouse.h>
#include <Novot.h>
#include <PropertyManager.h>
#include <lua.hpp>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <d3d11.h>
#include <Windows.h>
#include <DxLib.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <utility>

namespace Game {
	namespace {
		std::string ReadString(lua_State* state, int table, const char* key,
			const std::string& fallback = "") {
			lua_getfield(state, table, key);
			const char* value = lua_tostring(state, -1);
			const std::string result = value ? value : fallback;
			lua_pop(state, 1);
			return result;
		}

		int ReadInteger(lua_State* state, int table, const char* key, int fallback = 0) {
			lua_getfield(state, table, key);
			const int result = lua_isnumber(state, -1)
				? static_cast<int>(lua_tointeger(state, -1)) : fallback;
			lua_pop(state, 1);
			return result;
		}

		double ReadNumber(lua_State* state, int table, const char* key, double fallback = 0.0) {
			lua_getfield(state, table, key);
			const double result = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : fallback;
			lua_pop(state, 1);
			return result;
		}

		bool ReadBool(lua_State* state, int table, const char* key, bool fallback) {
			lua_getfield(state, table, key);
			const bool result = lua_isboolean(state, -1) ? lua_toboolean(state, -1) != 0 : fallback;
			lua_pop(state, 1);
			return result;
		}

		std::string EscapeLua(const std::string& value) {
			std::string escaped;
			for (const char character : value) {
				switch (character) {
				case '\\': escaped += "\\\\"; break;
				case '"': escaped += "\\\""; break;
				case '\n': escaped += "\\n"; break;
				case '\r': escaped += "\\r"; break;
				default: escaped += character; break;
				}
			}
			return escaped;
		}

		template<size_t size>
		void CopyToBuffer(std::array<char, size>& buffer, const std::string& value) {
			buffer.fill('\0');
			strncpy_s(buffer.data(), buffer.size(), value.c_str(), buffer.size() - 1);
		}

		class MapEditorOverlay : public Scene {
		protected:
			void _Draw() const override {
				if (not ImGui::GetCurrentContext()) return;
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			}
		};
	}

	void MapEditor::_Load() {
		if (not ImGui::GetCurrentContext()) {
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImGui_ImplWin32_Init(DxLib::GetMainWindowHandle());
			ImGui_ImplDX11_Init(
				static_cast<ID3D11Device*>(const_cast<void*>(DxLib::GetUseDirect3D11Device())),
				static_cast<ID3D11DeviceContext*>(const_cast<void*>(DxLib::GetUseDirect3D11DeviceContext())));
		}
		Camera.SetTracking({});
		Camera.SetPosition({ 0.0, 0.0 });
		Camera.SetZoom(1.0);
		CreateScene<MapEditorOverlay>()->SetUIMode(true)->IgnoreCamera();
		if (not LoadMap(_mapPath)) {
			_tiles.resize(_width * _height);
			_status = "Could not load board/default.lua; create or select a valid Lua map file.";
			RebuildTileSprites();
		}
		CopyToBuffer(_mapPathInput, _mapPath);
		CopyToBuffer(_tileTextureInput, _tileTexture);
		CopyToBuffer(_tileBrushInput, _tileTexture);

		CreateTimer(1.0, [this]() {
			PropertyManager.ToFullScreen();
			});
	}

	void MapEditor::_Release() {
		if (ImGui::GetCurrentContext()) {
			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}
		_tileSprites.clear();
		_objectSprites.clear();
		_characterSprites.clear();
		_characterLabels.clear();
	}

	void MapEditor::_Update(float) {
		if (not ImGui::GetCurrentContext()) return;
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		UpdateEditorUI();
		HandleViewportInput();
	}

	void MapEditor::_Draw() const {
		if (not ImGui::GetCurrentContext()) return;
		if (not (_selectedX < 0) && not (_selectedY < 0)) {
			const auto topLeft = Camera.WorldToScreen(TilePosition(_selectedX, _selectedY));
			const auto bottomRight = Camera.WorldToScreen(
				TilePosition(_selectedX + 1, _selectedY + 1));
			ImGui::GetBackgroundDrawList()->AddRectFilled(
				ImVec2(static_cast<float>(topLeft.x), static_cast<float>(topLeft.y)),
				ImVec2(static_cast<float>(bottomRight.x), static_cast<float>(bottomRight.y)),
				IM_COL32(255, 255, 0, 150));
		}
		ImGui::Render();
	}

	bool MapEditor::LoadMap(const std::string& path) {
		lua_State* state = luaL_newstate();
		if (not state) return false;
		luaL_openlibs(state);
		const auto resolvedPath = AssetManager.ResolvePath(path);
		if (luaL_dofile(state, resolvedPath.string().c_str()) != LUA_OK) {
			lua_close(state);
			return false;
		}
		lua_getglobal(state, "board");
		if (not lua_istable(state, -1)) {
			lua_close(state);
			return false;
		}

		const int board = lua_gettop(state);
		const int width = ReadInteger(state, board, "width", 9);
		const int height = ReadInteger(state, board, "height", 7);
		if (width <= 0 || height <= 0 || 256 < width || 256 < height) {
			lua_close(state);
			return false;
		}
		_width = width;
		_height = height;
		_tileSize = ReadInteger(state, board, "tileSize", 72);
		_tileSize = Clamp(_tileSize, 8, 512);
		lua_getfield(state, board, "position");
		if (lua_istable(state, -1)) {
			_boardPosition.x = ReadNumber(state, -1, "x", 320.0);
			_boardPosition.y = ReadNumber(state, -1, "y", 48.0);
		}
		lua_pop(state, 1);

		std::map<std::string, std::string> terrainTextures;
		lua_getfield(state, board, "textures");
		if (lua_istable(state, -1)) {
			for (const char* key : { "normal", "disabled", "other" }) {
				const auto value = ReadString(state, -1, key);
				if (value[0] != '\0') terrainTextures[key] = value;
			}
		}
		lua_pop(state, 1);
		const auto defaultTexture = terrainTextures.find("normal");
		_defaultTileTexture = defaultTexture == terrainTextures.end() ? "" : defaultTexture->second;
		_tileTexture = _defaultTileTexture;
		_tiles.assign(_width * _height, {});
		lua_getfield(state, board, "tiles");
		if (lua_istable(state, -1)) {
			for (int index = 0; index < _width * _height; ++index) {
				lua_rawgeti(state, -1, index + 1);
				auto& tile = _tiles[index];
				if (lua_isstring(state, -1)) {
					const std::string terrain = lua_tostring(state, -1);
					tile.passable = terrain != "disabled";
					const auto texture = terrainTextures.find(terrain);
					if (texture != terrainTextures.end()) tile.texture = texture->second;
				}
				else if (lua_istable(state, -1)) {
					const std::string terrain = ReadString(state, -1, "terrain", "normal");
					const std::string explicitTexture = ReadString(state, -1, "texture");
					tile.passable = ReadBool(state, -1, "passable", terrain != "disabled");
					const auto texture = terrainTextures.find(terrain);
					tile.texture = explicitTexture.empty()
						? (texture == terrainTextures.end() ? std::string() : texture->second)
						: explicitTexture;
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);
		if (_defaultTileTexture.empty() && not _tiles.empty()) _defaultTileTexture = _tiles.front().texture;

		_objects.clear();
		lua_getfield(state, board, "objects");
		if (lua_istable(state, -1)) {
			const int count = static_cast<int>(lua_rawlen(state, -1));
			for (int index = 1; index <= count; ++index) {
				lua_rawgeti(state, -1, index);
				if (lua_istable(state, -1)) {
					_objects.push_back({
						ReadString(state, -1, "texture"),
						ReadInteger(state, -1, "x"),
						ReadInteger(state, -1, "y"),
						ReadInteger(state, -1, "width", 1),
						ReadInteger(state, -1, "height", 1)
					});
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);

		_characterCatalog.clear();
		lua_getfield(state, board, "characterCatalog");
		if (lua_istable(state, -1)) {
			const int catalog = lua_gettop(state);
			lua_pushnil(state);
			while (lua_next(state, catalog) != 0) {
				if (lua_type(state, -2) == LUA_TSTRING && lua_istable(state, -1)) {
					const auto type = ParseBoardCharacterType(lua_tostring(state, -2));
					const bool exists = std::any_of(_characterCatalog.begin(), _characterCatalog.end(),
						[type](const auto& entry) { return entry.type == type; });
					if (not exists) {
						_characterCatalog.push_back({ type,
							ReadString(state, -1, "displayName"),
							ReadString(state, -1, "icon") });
					}
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);
		_characters.clear();
		lua_getfield(state, board, "units");
		if (lua_istable(state, -1)) {
			const int count = static_cast<int>(lua_rawlen(state, -1));
			for (int index = 1; index <= count; ++index) {
				lua_rawgeti(state, -1, index);
				if (lua_istable(state, -1)) {
					const auto team = std::string(ReadString(state, -1, "team", "ally"));
					const auto type = ParseBoardCharacterType(ReadString(state, -1, "character", "plea"));
					const auto typeName = BoardCharacterTypeKey(type);
					const auto catalogEntry = std::find_if(_characterCatalog.begin(), _characterCatalog.end(),
						[type](const auto& entry) { return entry.type == type; });
					const std::string icon = ReadString(state, -1, "icon",
						ReadString(state, -1, "texture", catalogEntry == _characterCatalog.end() ? "" : catalogEntry->icon));
					_characters.push_back({
						ReadString(state, -1, "id"),
						type,
						ReadString(state, -1, "displayName", catalogEntry == _characterCatalog.end() ? typeName : catalogEntry->displayName),
						icon,
						team == "enemy" ? BoardTeam::Enemy : BoardTeam::Ally,
						ReadString(state, -1, "move", "adjacent"),
						ReadInteger(state, -1, "x"),
						ReadInteger(state, -1, "y")
					});
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);
		lua_close(state);
		_mapPath = path;
		_selectedX = _selectedY = -1;
		_selectedObject = -1;
		_isDirty = false;
		CopyToBuffer(_mapPathInput, _mapPath);
		CopyToBuffer(_tileTextureInput, _tileTexture);
		CopyToBuffer(_tileBrushInput, _tileTexture);
		CopyToBuffer(_defaultTextureInput, _defaultTileTexture);
		RebuildTileSprites();
		RebuildObjectSprites();
		RebuildCharacterSprites();
		return true;
	}

	bool MapEditor::SaveMap() {
		if (std::any_of(_tiles.begin(), _tiles.end(), [](const auto& tile) { return tile.texture.empty(); })) {
			_status = "Every tile needs a texture before the map can be saved.";
			return false;
		}
		const std::filesystem::path outputPath = AssetManager.ResolvePath(_mapPath);
		std::error_code error;
		std::filesystem::create_directories(outputPath.parent_path(), error);
		std::ofstream output(outputPath, std::ios::binary);
		if (not output) {
			_status = "Could not open map file for writing.";
			return false;
		}
		output << "board = {\n";
		output << "    width = " << _width << ",\n";
		output << "    height = " << _height << ",\n";
		output << "    tileSize = " << _tileSize << ",\n";
		output << "    position = { x = " << _boardPosition.x << ", y = " << _boardPosition.y << " },\n";
		output << "    textures = { normal = \"" << EscapeLua(_defaultTileTexture)
			<< "\", disabled = \"" << EscapeLua(_defaultTileTexture)
			<< "\", other = \"" << EscapeLua(_defaultTileTexture) << "\" },\n";
		output << "    characterCatalog = {\n";
		for (const auto& entry : _characterCatalog) {
			output << "        [\"" << BoardCharacterTypeKey(entry.type) << "\"] = { displayName = \""
				<< EscapeLua(entry.displayName) << "\", icon = \"" << EscapeLua(entry.icon) << "\" },\n";
		}
		output << "    },\n    tiles = {\n";
		for (const auto& tile : _tiles) {
			output << "        { terrain = \"" << (tile.passable ? "normal" : "disabled")
				<< "\", passable = " << (tile.passable ? "true" : "false")
				<< ", texture = \"" << EscapeLua(tile.texture) << "\" },\n";
		}
		output << "    },\n    objects = {\n";
		for (const auto& object : _objects) {
			output << "        { texture = \"" << EscapeLua(object.texture) << "\", x = " << object.x
				<< ", y = " << object.y << ", width = " << object.width
				<< ", height = " << object.height << " },\n";
		}
		output << "    },\n    units = {\n";
		for (const auto& character : _characters) {
			output << "        { id = \"" << EscapeLua(character.id) << "\", character = \""
				<< BoardCharacterTypeKey(character.type) << "\", displayName = \""
				<< EscapeLua(character.displayName) << "\", icon = \"" << EscapeLua(character.icon)
				<< "\", team = \"" << (character.team == BoardTeam::Ally ? "ally" : "enemy")
				<< "\", move = \"" << EscapeLua(character.moveType) << "\", x = "
				<< character.x << ", y = " << character.y << " },\n";
		}
		output << "    }\n}\n";
		if (not output.good()) {
			_status = "Map file write failed.";
			return false;
		}
		_isDirty = false;
		_status = "Saved " + outputPath.filename().string();
		return true;
	}

	void MapEditor::ResizeMap(int width, int height) {
		width = Clamp(width, 1, 256);
		height = Clamp(height, 1, 256);
		if (width == _width && height == _height) return;
		std::vector<MapEditorTileData> resized(width * height, MapEditorTileData{ _defaultTileTexture, true });
		for (int y = 0; y < Min(_height, height); ++y) {
			for (int x = 0; x < Min(_width, width); ++x) {
				resized[y * width + x] = _tiles[y * _width + x];
			}
		}
		_width = width;
		_height = height;
		_tiles = std::move(resized);
		_selectedX = _selectedY = -1;
		_selectedObject = -1;
		std::erase_if(_objects, [this](const auto& object) {
			return object.x < 0 || object.y < 0 || not (object.x < _width) || not (object.y < _height);
		});
		for (auto& object : _objects) {
			object.width = Min(object.width, _width - object.x);
			object.height = Min(object.height, _height - object.y);
		}
		std::erase_if(_characters, [this](const auto& character) {
			return character.x < 0 || character.y < 0 || not (character.x < _width) || not (character.y < _height);
		});
		_isDirty = true;
		RebuildTileSprites();
		RebuildObjectSprites();
		RebuildCharacterSprites();
	}

	void MapEditor::RebuildTileSprites() {
		for (const auto& sprite : _tileSprites) if (sprite) DeleteScene(sprite);
		_tileSprites.clear();
		for (int y = 0; y < _height; ++y) {
			for (int x = 0; x < _width; ++x) {
				const auto& data = _tiles[y * _width + x];
				if (data.texture.empty()) continue;
				const auto& sprite = CreateScene<Sprite2D>();
				sprite->SetTexture(LoadAsset<Texture>(data.texture));
				const auto textureSize = sprite->GetRawSize();
				if (not (textureSize.x <= 0.0) && not (textureSize.y <= 0.0)) {
					sprite->SetScale({ _tileSize / textureSize.x, _tileSize / textureSize.y });
				}
				if (not data.passable) sprite->SetModulate({ 100, 100, 100 });
				sprite->SetPosition(TilePosition(x, y));
				_tileSprites.push_back(sprite);
			}
		}
	}

	void MapEditor::RebuildObjectSprites() {
		for (const auto& sprite : _objectSprites) if (sprite) DeleteScene(sprite);
		_objectSprites.clear();
		for (const auto& object : _objects) {
			if (object.texture.empty()) continue;
			const auto& sprite = CreateScene<Sprite2D>();
			sprite->SetTexture(LoadAsset<Texture>(object.texture));
			const auto textureSize = sprite->GetRawSize();
			const Vector2D targetSize(_tileSize * object.width, _tileSize * object.height);
			if (not (textureSize.x <= 0.0) && not (textureSize.y <= 0.0)) {
				sprite->SetScale({ targetSize.x / textureSize.x, targetSize.y / textureSize.y });
			}
			sprite->SetPosition(TilePosition(object.x, object.y));
			_objectSprites.push_back(sprite);
		}
	}

	void MapEditor::RebuildCharacterSprites() {
		for (const auto& sprite : _characterSprites) if (sprite) DeleteScene(sprite);
		for (const auto& label : _characterLabels) if (label) DeleteScene(label);
		_characterSprites.clear();
		_characterLabels.clear();
		for (const auto& character : _characters) {
			if (not character.icon.empty()) {
				const auto& sprite = CreateScene<Sprite2D>();
				sprite->SetTexture(LoadAsset<Texture>(character.icon));
				const auto textureSize = sprite->GetRawSize();
				if (not (textureSize.x <= 0.0) && not (textureSize.y <= 0.0)) {
					const auto scale = Min(_tileSize / textureSize.x, _tileSize / textureSize.y) * 0.8;
					sprite->SetScale(scale);
				}
				sprite->SetPosition(TilePosition(character.x, character.y) + Vector2D(_tileSize * 0.1, _tileSize * 0.05));
				_characterSprites.push_back(sprite);
			}
			const auto& label = CreateScene<TextLabel>();
			label->SetFontSize(13)->SetText(character.displayName)->SetWidth(_tileSize);
			label->SetForegroundColor({ 255, 255, 255 });
			label->SetPosition(TilePosition(character.x, character.y) + Vector2D(2.0, _tileSize * 0.76));
			_characterLabels.push_back(label);
		}
	}

	void MapEditor::UpdateEditorUI() {
		ImGui::SetNextWindowPos(ImVec2(900.0f, 12.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(360.0f, 690.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Board Map Editor")) {
			ImGui::InputText("Lua map file", _mapPathInput.data(), _mapPathInput.size());
			if (ImGui::Button("Load")) {
				if (LoadMap(_mapPathInput.data())) _status = "Map loaded.";
				else _status = "Could not load map file.";
			}
			ImGui::SameLine();
			if (ImGui::Button("Save")) {
				_mapPath = _mapPathInput.data();
				SaveMap();
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(_isDirty ? "Modified" : "Saved");
			float boardPosition[2] = { static_cast<float>(_boardPosition.x), static_cast<float>(_boardPosition.y) };
			if (ImGui::InputFloat2("Board position", boardPosition)) {
				_boardPosition = { boardPosition[0], boardPosition[1] };
				_isDirty = true;
				RebuildTileSprites();
				RebuildObjectSprites();
				RebuildCharacterSprites();
			}
			int width = _width;
			int height = _height;
			const bool widthChanged = ImGui::InputInt("Tiles wide", &width);
			const bool heightChanged = ImGui::InputInt("Tiles high", &height);
			if (widthChanged || heightChanged) ResizeMap(width, height);
			int tileSize = _tileSize;
			if (ImGui::InputInt("Tile size", &tileSize)) {
				_tileSize = Clamp(tileSize, 8, 512);
				_isDirty = true;
				RebuildTileSprites();
				RebuildObjectSprites();
				RebuildCharacterSprites();
			}
			ImGui::Separator();
			const char* tools[] = { "Select tile", "Paint tile", "Place object", "Place character" };
			int selectedTool = static_cast<int>(_tool);
			if (ImGui::Combo("Tool", &selectedTool, tools, 4)) _tool = static_cast<MapEditorTool>(selectedTool);
			if (not (_selectedX < 0) && not (_selectedY < 0)) {
				ImGui::Text("Selected tile: %d, %d", _selectedX, _selectedY);
				auto& tile = _tiles[_selectedY * _width + _selectedX];
				if (ImGui::InputText("Tile texture", _tileTextureInput.data(), _tileTextureInput.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
					tile.texture = _tileTextureInput.data();
					_isDirty = true;
					RebuildTileSprites();
				}
				if (ImGui::Checkbox("Passable", &tile.passable)) {
					_tilePassable = tile.passable;
					_isDirty = true;
					RebuildTileSprites();
				}
			}
			ImGui::SeparatorText("Tile brush");
			ImGui::InputText("Brush texture", _tileBrushInput.data(), _tileBrushInput.size());
			ImGui::Checkbox("Brush passable", &_tilePassable);
			if (ImGui::InputText("New tile default texture", _defaultTextureInput.data(), _defaultTextureInput.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
				_defaultTileTexture = _defaultTextureInput.data();
				_isDirty = true;
			}
			ImGui::SeparatorText("Objects spanning tiles");
			ImGui::InputText("Object texture", _objectTextureInput.data(), _objectTextureInput.size());
			ImGui::InputInt("Object tile width", &_objectWidth);
			ImGui::InputInt("Object tile height", &_objectHeight);
			_objectWidth = Clamp(_objectWidth, 1, 256);
			_objectHeight = Clamp(_objectHeight, 1, 256);
			if (not (_selectedObject < 0) && _selectedObject < static_cast<int>(_objects.size())) {
				auto& object = _objects[_selectedObject];
				ImGui::Text("Selected object: %d", _selectedObject + 1);
				if (ImGui::InputText("Selected object texture", _objectTextureInput.data(), _objectTextureInput.size())) {
					object.texture = _objectTextureInput.data();
					_isDirty = true;
					RebuildObjectSprites();
				}
				int objectWidth = object.width;
				int objectHeight = object.height;
				if (ImGui::InputInt("Selected width", &objectWidth)) {
					object.width = Clamp(objectWidth, 1, _width - object.x);
					_isDirty = true;
					RebuildObjectSprites();
				}
				if (ImGui::InputInt("Selected height", &objectHeight)) {
					object.height = Clamp(objectHeight, 1, _height - object.y);
					_isDirty = true;
					RebuildObjectSprites();
				}
				if (ImGui::Button("Delete selected object")) {
					_objects.erase(_objects.begin() + _selectedObject);
					_selectedObject = -1;
					_isDirty = true;
					RebuildObjectSprites();
				}
			}
			ImGui::SeparatorText("Characters");
			if (not _characterCatalog.empty()) {
				std::vector<std::string> names;
				std::vector<const char*> labels;
				for (const auto& info : _characterCatalog) {
				names.push_back(info.displayName + " (" + BoardCharacterTypeKey(info.type) + ")");
				}
				for (const auto& name : names) labels.push_back(name.c_str());
				_selectedCharacter = Clamp(_selectedCharacter, 0, static_cast<int>(labels.size()) - 1);
				if (ImGui::Combo("Character", &_selectedCharacter, labels.data(), static_cast<int>(labels.size()))) {
					_catalogSelection = -1;
				}
				if (_catalogSelection != _selectedCharacter) {
					CopyToBuffer(_characterNameInput, _characterCatalog[_selectedCharacter].displayName);
					CopyToBuffer(_characterIconInput, _characterCatalog[_selectedCharacter].icon);
					_catalogSelection = _selectedCharacter;
				}
				auto& characterInfo = _characterCatalog[_selectedCharacter];
				if (ImGui::InputText("Display name", _characterNameInput.data(), _characterNameInput.size())) {
					characterInfo.displayName = _characterNameInput.data();
					for (auto& placed : _characters) {
						if (placed.type == characterInfo.type) placed.displayName = characterInfo.displayName;
					}
					_isDirty = true;
					RebuildCharacterSprites();
				}
				if (ImGui::InputText("Character icon", _characterIconInput.data(), _characterIconInput.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
					characterInfo.icon = _characterIconInput.data();
					for (auto& placed : _characters) {
						if (placed.type == characterInfo.type) placed.icon = characterInfo.icon;
					}
					_isDirty = true;
					RebuildCharacterSprites();
				}
				ImGui::RadioButton("Ally", &_characterTeam, 0);
				ImGui::SameLine();
				ImGui::RadioButton("Enemy", &_characterTeam, 1);
				if (ImGui::Button("Remove last character")) {
					if (not _characters.empty()) {
						_characters.pop_back();
						_isDirty = true;
						RebuildCharacterSprites();
					}
				}
				ImGui::TextWrapped("Select Place character, then click a tile. Allies can be placed once per character type; enemies can be repeated.");
			}
			ImGui::Separator();
			ImGui::TextWrapped("Right mouse drag: pan | Mouse wheel: zoom at cursor");
			if (not _status.empty()) ImGui::TextWrapped("%s", _status.c_str());
		}
		ImGui::End();
	}

	void MapEditor::HandleViewportInput() {
		if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
			_isPanning = false;
			_leftMouseDown = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
			return;
		}
		int mouseX = 0;
		int mouseY = 0;
		GetMousePoint(&mouseX, &mouseY);
		const Vector2D screen(static_cast<double>(mouseX), static_cast<double>(mouseY));
		const auto wheel = Mouse.GetMouseEvent().scrollOffset;
		const double wheelDelta = wheel.y != 0.0 ? wheel.y : wheel.x;
		if (wheelDelta != 0.0) {
			const auto worldBefore = Camera.ScreenToWorld(screen);
			const auto zoom = Clamp(Camera.GetZoom().x * (0.0 < wheelDelta ? 1.1 : 0.9), 0.25, 4.0);
			Camera.SetZoom(zoom);
			Camera.SetPosition(Camera.GetPosition() + worldBefore - Camera.ScreenToWorld(screen));
		}
		const bool rightDown = (GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;
		if (rightDown) {
			if (_isPanning) Camera.SetPosition(Camera.GetPosition() - (screen - _lastMouseScreen) / Camera.GetZoom());
			_lastMouseScreen = screen;
		}
		_isPanning = rightDown;
		const bool leftDown = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
		const bool leftPressed = leftDown && not _leftMouseDown;
		_leftMouseDown = leftDown;
		if (not leftPressed) return;
		const auto world = Camera.ScreenToWorld(screen);
		const int x = static_cast<int>(std::floor((world.x - _boardPosition.x) / _tileSize));
		const int y = static_cast<int>(std::floor((world.y - _boardPosition.y) / _tileSize));
		if (x < 0 || y < 0 || not (x < _width) || not (y < _height)) return;
		if (_tool == MapEditorTool::Select) SelectAt(x, y);
		else PlaceAt(x, y);
	}

	void MapEditor::PlaceAt(int x, int y) {
		if (_tool == MapEditorTool::PaintTile) {
			auto& tile = _tiles[y * _width + x];
			tile.texture = _tileBrushInput.data();
			tile.passable = _tilePassable;
			_selectedX = x;
			_selectedY = y;
			_isDirty = true;
			RebuildTileSprites();
			return;
		}
		if (_tool == MapEditorTool::PlaceObject) {
			if (_objectTextureInput[0] == '\0') {
				_status = "Set an object texture before placing it.";
				return;
			}
			_objects.push_back({ _objectTextureInput.data(), x, y,
				Min(_objectWidth, _width - x), Min(_objectHeight, _height - y) });
			_selectedObject = static_cast<int>(_objects.size()) - 1;
			_isDirty = true;
			RebuildObjectSprites();
			return;
		}
		if (_tool == MapEditorTool::PlaceCharacter && not (_selectedCharacter < 0)
			&& _selectedCharacter < static_cast<int>(_characterCatalog.size())) {
			const auto& info = _characterCatalog[_selectedCharacter];
			const auto team = _characterTeam == 0 ? BoardTeam::Ally : BoardTeam::Enemy;
			if (team == BoardTeam::Ally && IsAllyTypePlaced(BoardCharacterTypeKey(info.type))) {
				_status = "This ally already has an initial tile.";
				return;
			}
			MapEditorCharacterData character;
			character.id = std::string(team == BoardTeam::Ally ? "ally_" : "enemy_") + std::to_string(_characters.size() + 1);
			character.type = info.type;
			character.displayName = info.displayName;
			character.icon = info.icon;
			character.team = team;
			character.x = x;
			character.y = y;
			_characters.push_back(character);
			_isDirty = true;
			RebuildCharacterSprites();
		}
	}

	void MapEditor::SelectAt(int x, int y) {
		_selectedX = x;
		_selectedY = y;
		_selectedObject = -1;
		const auto world = Vector2D(_boardPosition.x + x * _tileSize + _tileSize / 2.0,
			_boardPosition.y + y * _tileSize + _tileSize / 2.0);
		for (int index = static_cast<int>(_objects.size()) - 1; not (index < 0); --index) {
			const auto& object = _objects[index];
			const auto left = _boardPosition.x + object.x * _tileSize;
			const auto top = _boardPosition.y + object.y * _tileSize;
			if (not (world.x < left) && world.x < left + object.width * _tileSize
				&& not (world.y < top) && world.y < top + object.height * _tileSize) {
				_selectedObject = index;
				CopyToBuffer(_objectTextureInput, object.texture);
				break;
			}
		}
		const auto& tile = _tiles[y * _width + x];
		_tileTexture = tile.texture;
		_tilePassable = tile.passable;
		CopyToBuffer(_tileTextureInput, _tileTexture);
	}

	bool MapEditor::IsAllyTypePlaced(const std::string& type) const {
		const auto parsedType = ParseBoardCharacterType(type);
		return std::any_of(_characters.begin(), _characters.end(), [parsedType](const auto& character) {
			return character.team == BoardTeam::Ally && character.type == parsedType;
		});
	}

	Vector2D MapEditor::TilePosition(int x, int y) const {
		return _boardPosition + Vector2D(x * _tileSize, y * _tileSize);
	}
}
