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
#include <cctype>
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
			auto& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

			const auto fontPath = AssetManager.ResolvePath("font/JF-Dot-jiskan16.ttf");
			std::error_code error;
			if (std::filesystem::exists(fontPath, error)) {
				io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
			}

			ImGui_ImplWin32_Init(DxLib::GetMainWindowHandle());
			ImGui_ImplDX11_Init(
				static_cast<ID3D11Device*>(const_cast<void*>(DxLib::GetUseDirect3D11Device())),
				static_cast<ID3D11DeviceContext*>(const_cast<void*>(DxLib::GetUseDirect3D11DeviceContext())));
		}
		Camera.SetTracking({});
		Camera.SetPosition({ 0.0, 0.0 });
		Camera.SetZoom(1.0);
		CreateScene<MapEditorOverlay>()->SetUIMode(true)->IgnoreCamera();
		RefreshAssetFiles();
		const std::string lastPath = LoadLastOpenedMapPath();
		const std::string targetPath = lastPath.empty() ? _mapPath : lastPath;
		if (not LoadMap(targetPath)) {
			if (targetPath != _mapPath && LoadMap(_mapPath)) {
			}
			else {
				_tiles.resize(_width * _height);
				_status = "Luaマップファイルを読み込めませんでした。有効なLuaマップファイルを選択してください。";
				RebuildTileSprites();
			}
		}
		CopyToBuffer(_mapPathInput, _mapPath);
		CopyToBuffer(_tileTextureInput, _tileTexture);
		CopyToBuffer(_tileBrushInput, _tileTexture);

		CreateTimer(1.0, [this]() {
			PropertyManager.ToFullScreen();
			});
	}

	void MapEditor::_Release() {
		ClearThumbnailCache();
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
		int mouseX = 0;
		int mouseY = 0;
		GetMousePoint(&mouseX, &mouseY);
		const int mouseButtons = GetMouseInput();
		auto& io = ImGui::GetIO();
		io.AddMousePosEvent(static_cast<float>(mouseX), static_cast<float>(mouseY));
		io.AddMouseButtonEvent(0, (mouseButtons & MOUSE_INPUT_LEFT) != 0);
		io.AddMouseButtonEvent(1, (mouseButtons & MOUSE_INPUT_RIGHT) != 0);
		io.AddMouseButtonEvent(2, (mouseButtons & MOUSE_INPUT_MIDDLE) != 0);
		ImGui::NewFrame();
		UpdateEditorUI();
		HandleViewportInput();
	}

	void MapEditor::_Draw() const {
		if (not ImGui::GetCurrentContext()) return;
		auto* drawList = ImGui::GetBackgroundDrawList();
		auto drawTileHighlight = [this, drawList](int x, int y, int alpha) {
			const auto topLeft = Camera.WorldToScreen(TilePosition(x, y));
			const auto bottomRight = Camera.WorldToScreen(TilePosition(x + 1, y + 1));
			drawList->AddRectFilled(
				ImVec2(static_cast<float>(topLeft.x), static_cast<float>(topLeft.y)),
				ImVec2(static_cast<float>(bottomRight.x), static_cast<float>(bottomRight.y)),
				IM_COL32(255, 255, 0, alpha));
		};

		const bool hasSelection = not (_selectedX < 0) && not (_selectedY < 0);
		int hoveredX = -1;
		int hoveredY = -1;
		const auto mousePosition = ImGui::GetIO().MousePos;
		if (not (mousePosition.x < 0.0f) && not (mousePosition.y < 0.0f)) {
			const auto world = Camera.ScreenToWorld({ mousePosition.x, mousePosition.y });
			hoveredX = static_cast<int>(std::floor((world.x - _boardPosition.x) / _tileSize));
			hoveredY = static_cast<int>(std::floor((world.y - _boardPosition.y) / _tileSize));
			if (hoveredX < 0 || hoveredY < 0 || not (hoveredX < _width) || not (hoveredY < _height)) {
				hoveredX = hoveredY = -1;
			}
		}

		if (hasSelection) drawTileHighlight(_selectedX, _selectedY, 150);
		const bool hoverMatchesSelection = hasSelection
			&& hoveredX == _selectedX && hoveredY == _selectedY;
		if (not (hoveredX < 0) && not (hoveredY < 0) && not hoverMatchesSelection) {
			drawTileHighlight(hoveredX, hoveredY, 50);
		}
		if (not (_selectedObject < 0) && _selectedObject < static_cast<int>(_objects.size())) {
			const auto& object = _objects[_selectedObject];
			const auto pos = TilePosition(object.x, object.y);
			double objW = static_cast<double>(_tileSize);
			double objH = static_cast<double>(_tileSize);
			if (_selectedObject < static_cast<int>(_objectSprites.size()) && _objectSprites[_selectedObject]) {
				const auto rawSize = _objectSprites[_selectedObject]->GetRawSize();
				if (0.0 < rawSize.x && 0.0 < rawSize.y) {
					objW = rawSize.x * object.scaleX;
					objH = rawSize.y * object.scaleY;
				}
			}
			const auto topLeft = Camera.WorldToScreen(pos);
			const auto bottomRight = Camera.WorldToScreen(pos + Vector2D(objW, objH));
			drawList->AddRect(
				ImVec2(static_cast<float>(topLeft.x), static_cast<float>(topLeft.y)),
				ImVec2(static_cast<float>(bottomRight.x), static_cast<float>(bottomRight.y)),
				IM_COL32(0, 220, 255, 220), 0.0f, 0, 2.0f);
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
					const double scaleVal = ReadNumber(state, -1, "scale", 1.0);
					_objects.push_back({
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
		// マップのカタログにないキャラも配置できるよう、組み込みの既定カタログで補う
		// （マップ側の定義がある場合はそちらが優先される）
		const std::vector<MapEditorCharacterInfo> defaultCatalog = {
			{ BoardCharacterType::Plea, "プレア", "chara/plea/icon.png" },
			{ BoardCharacterType::Reko, "レコ", "chara/reko/icon.png" },
			{ BoardCharacterType::Zon, "ゾン", "chara/zon/icon.png" },
			{ BoardCharacterType::Gou, "ゴウ", "chara/gou/icon.png" },
			{ BoardCharacterType::Slime, "スライム", "enemy/slime/icon.png" },
			{ BoardCharacterType::Witch, "ウィッチ", "enemy/witch/icon.png" },
			{ BoardCharacterType::Golem, "ゴーレム", "enemy/golem/icon.png" },
			{ BoardCharacterType::RareSlime, "レアスライム", "enemy/slime/icon.png" },
			{ BoardCharacterType::RareWitch, "レアウィッチ", "enemy/witch/icon.png" },
			{ BoardCharacterType::RareGolem, "レアゴーレム", "enemy/golem/icon.png" }
		};
		for (const auto& entry : defaultCatalog) {
			const bool exists = std::any_of(_characterCatalog.begin(), _characterCatalog.end(),
				[&entry](const auto& placed) { return placed.type == entry.type; });
			if (not exists) _characterCatalog.push_back(entry);
		}
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
					MapEditorCharacterData character = {
						ReadString(state, -1, "id"),
						type,
						ReadString(state, -1, "displayName", catalogEntry == _characterCatalog.end() ? typeName : catalogEntry->displayName),
						icon,
						team == "enemy" ? BoardTeam::Enemy : BoardTeam::Ally,
						ReadString(state, -1, "move", "adjacent"),
						ReadInteger(state, -1, "x"),
						ReadInteger(state, -1, "y")
					};
					// 占有マスは本体からの相対オフセットで保存されているので絶対座標に戻す
					lua_getfield(state, -1, "tiles");
					if (lua_istable(state, -1)) {
						const int tileCount = static_cast<int>(lua_rawlen(state, -1));
						for (int tileIndex = 1; tileIndex <= tileCount; ++tileIndex) {
							lua_rawgeti(state, -1, tileIndex);
							if (lua_istable(state, -1)) {
								const int extraX = character.x + ReadInteger(state, -1, "x");
								const int extraY = character.y + ReadInteger(state, -1, "y");
								if (not (extraX < 0) && not (extraY < 0) && extraX < _width && extraY < _height) {
									character.extraTiles.push_back({ extraX, extraY });
								}
							}
							lua_pop(state, 1);
						}
					}
					lua_pop(state, 1);
					lua_getfield(state, -1, "iconOffset");
					if (lua_istable(state, -1)) {
						character.iconOffsetX = ReadNumber(state, -1, "x");
						character.iconOffsetY = ReadNumber(state, -1, "y");
					}
					lua_pop(state, 1);
					character.iconScale = ReadNumber(state, -1, "iconScale", 1.0);
					_characters.push_back(character);
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);
		lua_close(state);
		_mapPath = path;
		_selectedX = _selectedY = -1;
		_selectedObject = -1;
		_occupyEditIndex = -1;
		_isDirty = false;
		CopyToBuffer(_mapPathInput, _mapPath);
		CopyToBuffer(_tileTextureInput, _tileTexture);
		CopyToBuffer(_tileBrushInput, _tileTexture);
		CopyToBuffer(_defaultTextureInput, _defaultTileTexture);
		RebuildTileSprites();
		RebuildObjectSprites();
		RebuildCharacterSprites();
		SaveLastOpenedMapPath(path);
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
				<< ", height = " << object.height
				<< ", scaleX = " << object.scaleX << ", scaleY = " << object.scaleY << " },\n";
		}
		output << "    },\n    units = {\n";
		for (const auto& character : _characters) {
			output << "        { id = \"" << EscapeLua(character.id) << "\", character = \""
				<< BoardCharacterTypeKey(character.type) << "\", displayName = \""
				<< EscapeLua(character.displayName) << "\", icon = \"" << EscapeLua(character.icon)
				<< "\", team = \"" << (character.team == BoardTeam::Ally ? "ally" : "enemy")
				<< "\", move = \"" << EscapeLua(character.moveType) << "\", x = "
				<< character.x << ", y = " << character.y;
			if (not character.extraTiles.empty()) {
				output << ", tiles = {";
				for (const auto& extra : character.extraTiles) {
					output << " { x = " << (extra.first - character.x) << ", y = " << (extra.second - character.y) << " },";
				}
				output << " }";
			}
			if (character.iconOffsetX != 0.0 || character.iconOffsetY != 0.0) {
				output << ", iconOffset = { x = " << character.iconOffsetX << ", y = " << character.iconOffsetY << " }";
			}
			if (character.iconScale != 1.0) {
				output << ", iconScale = " << character.iconScale;
			}
			output << " },\n";
		}
		output << "    }\n}\n";
		if (not output.good()) {
			_status = "マップファイルの書き込みに失敗しました。";
			return false;
		}
		_isDirty = false;
		_status = "保存しました: " + outputPath.filename().string();
		SaveLastOpenedMapPath(_mapPath);
		return true;
	}

	void MapEditor::SaveLastOpenedMapPath(const std::string& path) {
		if (path.empty()) return;
		const std::filesystem::path configPath = "_Tools/last_map.txt";
		std::error_code error;
		std::filesystem::create_directories(configPath.parent_path(), error);
		std::ofstream output(configPath);
		if (output.is_open()) {
			output << path;
		}
	}

	std::string MapEditor::LoadLastOpenedMapPath() const {
		const std::filesystem::path configPath = "_Tools/last_map.txt";
		std::ifstream input(configPath);
		if (not input.is_open()) {
			// 旧パスからのフォールバック
			const auto legacyPath = AssetManager.ResolvePath("board/last_map.txt");
			input.open(legacyPath);
		}
		if (input.is_open()) {
			std::string path;
			if (std::getline(input, path)) {
				while (not path.empty() && (path.back() == '\r' || path.back() == '\n' || path.back() == ' ')) {
					path.pop_back();
				}
				if (not path.empty()) {
					return path;
				}
			}
		}
		return "";
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
			sprite->SetScale({ object.scaleX, object.scaleY });
			sprite->SetPosition(TilePosition(object.x, object.y));
			_objectSprites.push_back(sprite);
		}
	}

	void MapEditor::RebuildCharacterSprites() {
		for (const auto& sprite : _characterSprites) if (sprite) DeleteScene(sprite);
		for (const auto& label : _characterLabels) if (label) DeleteScene(label);
		for (const auto& sprite : _extraTileSprites) if (sprite) DeleteScene(sprite);
		_characterSprites.clear();
		_characterLabels.clear();
		_extraTileSprites.clear();
		for (const auto& character : _characters) {
			if (not character.icon.empty()) {
				const auto& sprite = CreateScene<Sprite2D>();
				sprite->SetTexture(LoadAsset<Texture>(character.icon));
				const auto textureSize = sprite->GetRawSize();
				if (not (textureSize.x <= 0.0) && not (textureSize.y <= 0.0)) {
					const auto scale = Min(_tileSize / textureSize.x, _tileSize / textureSize.y) * 0.8 * character.iconScale;
					sprite->SetScale(scale);
				}
				sprite->SetPosition(TilePosition(character.x, character.y) + Vector2D(_tileSize * 0.1, _tileSize * 0.05)
					+ Vector2D(character.iconOffsetX, character.iconOffsetY));
				_characterSprites.push_back(sprite);
			}
			const auto& label = CreateScene<TextLabel>();
			label->SetFontSize(13)->SetText(character.displayName)->SetWidth(_tileSize);
			label->SetForegroundColor({ 0, 0, 0 });
			label->SetPosition(TilePosition(character.x, character.y) + Vector2D(2.0, _tileSize * 0.76));
			_characterLabels.push_back(label);
			// 占有マスは半透明の小さいアイコンで示す
			for (const auto& extra : character.extraTiles) {
				if (character.icon.empty()) continue;
				const auto& ghost = CreateScene<Sprite2D>();
				ghost->SetTexture(LoadAsset<Texture>(character.icon));
				const auto ghostSize = ghost->GetRawSize();
				if (not (ghostSize.x <= 0.0) && not (ghostSize.y <= 0.0)) {
					ghost->SetScale(Min(_tileSize / ghostSize.x, _tileSize / ghostSize.y) * 0.5);
				}
				ghost->alpha = 110;
				ghost->SetPosition(TilePosition(extra.first, extra.second) + Vector2D(_tileSize * 0.25, _tileSize * 0.25));
				_extraTileSprites.push_back(ghost);
			}
		}
	}

	void MapEditor::UpdateEditorUI() {
		ImGui::SetNextWindowPos(ImVec2(900.0f, 12.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(360.0f, 690.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("マップエディタ")) {
			ImGui::InputText("Luaマップファイル", _mapPathInput.data(), _mapPathInput.size());
			if (ImGui::Button("読み込み")) {
				if (LoadMap(_mapPathInput.data())) _status = "マップを読み込みました。";
				else _status = "マップファイルを読み込めませんでした。";
			}
			ImGui::SameLine();
			if (ImGui::Button("保存")) {
				_mapPath = _mapPathInput.data();
				SaveMap();
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(_isDirty ? "未保存" : "保存済み");
			float boardPosition[2] = { static_cast<float>(_boardPosition.x), static_cast<float>(_boardPosition.y) };
			if (ImGui::InputFloat2("盤面位置", boardPosition)) {
				_boardPosition = { boardPosition[0], boardPosition[1] };
				_isDirty = true;
				RebuildTileSprites();
				RebuildObjectSprites();
				RebuildCharacterSprites();
			}
			int width = _width;
			int height = _height;
			const bool widthChanged = ImGui::InputInt("タイル幅", &width);
			const bool heightChanged = ImGui::InputInt("タイル高", &height);
			if (widthChanged || heightChanged) ResizeMap(width, height);
			int tileSize = _tileSize;
			if (ImGui::InputInt("タイルサイズ", &tileSize)) {
				_tileSize = Clamp(tileSize, 8, 512);
				_isDirty = true;
				RebuildTileSprites();
				RebuildObjectSprites();
				RebuildCharacterSprites();
			}
			ImGui::Separator();
			const char* tools[] = { "タイル選択", "タイル配置", "オブジェクト配置", "キャラクター配置" };
			int selectedTool = static_cast<int>(_tool);
			if (ImGui::Combo("ツール", &selectedTool, tools, 4)) _tool = static_cast<MapEditorTool>(selectedTool);
			if (not (_selectedX < 0) && not (_selectedY < 0)) {
				ImGui::Text("選択タイル: %d, %d", _selectedX, _selectedY);
				auto& tile = _tiles[_selectedY * _width + _selectedX];
				if (ImGui::InputText("タイルテクスチャ", _tileTextureInput.data(), _tileTextureInput.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
					tile.texture = _tileTextureInput.data();
					_isDirty = true;
					RebuildTileSprites();
				}
				if (ImGui::Checkbox("通行可能", &tile.passable)) {
					_tilePassable = tile.passable;
					_isDirty = true;
					RebuildTileSprites();
				}
				// このタイルに置かれたオブジェクトの削除
				int objectsOnTile = 0;
				for (const auto& object : _objects) {
					if (object.x == _selectedX && object.y == _selectedY) objectsOnTile++;
				}
				if (0 < objectsOnTile) {
					ImGui::Text("このタイルのオブジェクト: %d件", objectsOnTile);
					if (ImGui::Button("このタイルのオブジェクトを削除")) {
						const bool hadSelectedObject = not (_selectedObject < 0);
						const auto selectedObjectData = hadSelectedObject ? _objects[_selectedObject] : MapEditorObjectData{};
						std::erase_if(_objects, [this](const MapEditorObjectData& object) {
							return object.x == _selectedX && object.y == _selectedY;
							});
						_selectedObject = -1;
						if (hadSelectedObject) {
							for (int index = 0; index < static_cast<int>(_objects.size()); ++index) {
								const auto& object = _objects[index];
								if (object.texture == selectedObjectData.texture && object.x == selectedObjectData.x
									&& object.y == selectedObjectData.y && object.width == selectedObjectData.width
									&& object.height == selectedObjectData.height && object.scaleX == selectedObjectData.scaleX
									&& object.scaleY == selectedObjectData.scaleY) {
									_selectedObject = index;
									break;
								}
							}
						}
						_isDirty = true;
						RebuildObjectSprites();
					}
				}
				// このタイルに置かれたキャラクターの削除
				for (int index = 0; index < static_cast<int>(_characters.size()); ++index) {
					auto& character = _characters[index];
					if (character.x != _selectedX || character.y != _selectedY) continue;
					ImGui::PushID(index);
					ImGui::Separator();
					ImGui::Text("%s (%s / %s)", character.displayName.c_str(), BoardCharacterTypeKey(character.type),
						character.team == BoardTeam::Ally ? "味方" : "敵");
					ImGui::Text("占有マス: %d", static_cast<int>(character.extraTiles.size()));
					float iconOffset[2] = { static_cast<float>(character.iconOffsetX), static_cast<float>(character.iconOffsetY) };
					if (ImGui::InputFloat2("アイコンオフセット", iconOffset)) {
						character.iconOffsetX = iconOffset[0];
						character.iconOffsetY = iconOffset[1];
						_isDirty = true;
						RebuildCharacterSprites();
					}
					float iconScale = static_cast<float>(character.iconScale);
					if (ImGui::InputFloat("アイコンスケール", &iconScale)) {
						character.iconScale = Max(static_cast<double>(iconScale), 0.05);
						_isDirty = true;
						RebuildCharacterSprites();
					}
					if (_occupyEditIndex == index) {
						ImGui::TextUnformatted("占有マス編集中: タイルをクリックで追加 / 削除");
						if (ImGui::Button("占有マス編集を終了")) {
							_occupyEditIndex = -1;
						}
					}
					else if (ImGui::Button("占有マスを編集")) {
						_occupyEditIndex = index;
						_tool = MapEditorTool::Select;
					}
					if (ImGui::Button("このキャラクターを削除")) {
						_characters.erase(_characters.begin() + index);
						_occupyEditIndex = -1;
						_isDirty = true;
						RebuildCharacterSprites();
						ImGui::PopID();
						break;
					}
					ImGui::PopID();
				}
			}
			ImGui::SeparatorText("タイルブラシ");
			ImGui::InputText("ブラシテクスチャ", _tileBrushInput.data(), _tileBrushInput.size());
			ImGui::Checkbox("ブラシ通行可能", &_tilePassable);
			if (ImGui::InputText("デフォルトタイルテクスチャ", _defaultTextureInput.data(), _defaultTextureInput.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
				_defaultTileTexture = _defaultTextureInput.data();
				_isDirty = true;
			}
			ImGui::SeparatorText("オブジェクト設定");
			ImGui::InputText("オブジェクトテクスチャ", _objectTextureInput.data(), _objectTextureInput.size());
			ImGui::Checkbox("縦横比固定", &_objectScaleUniform);
			if (_objectScaleUniform) {
				ImGui::DragFloat("配置スケール", &_objectScale, 0.05f, 0.05f, 20.0f, "%.2fx");
				_objectScale = static_cast<float>(Clamp(static_cast<double>(_objectScale), 0.05, 50.0));
			}
			else {
				float scaleInput[2] = { _objectScaleX, _objectScaleY };
				if (ImGui::DragFloat2("配置スケール (X, Y)", scaleInput, 0.05f, 0.05f, 20.0f, "%.2f")) {
					_objectScaleX = static_cast<float>(Clamp(static_cast<double>(scaleInput[0]), 0.05, 50.0));
					_objectScaleY = static_cast<float>(Clamp(static_cast<double>(scaleInput[1]), 0.05, 50.0));
				}
			}
			if (not (_selectedObject < 0) && _selectedObject < static_cast<int>(_objects.size())) {
				auto& object = _objects[_selectedObject];
				ImGui::Text("選択中オブジェクト: %d (タイル: %d, %d)", _selectedObject + 1, object.x, object.y);
				if (ImGui::InputText("選択中テクスチャ", _objectTextureInput.data(), _objectTextureInput.size())) {
					object.texture = _objectTextureInput.data();
					_isDirty = true;
					RebuildObjectSprites();
				}
				float selUniform = static_cast<float>(object.scaleX);
				if (ImGui::DragFloat("選択中スケール", &selUniform, 0.05f, 0.05f, 20.0f, "%.2fx")) {
					const double clamped = Clamp(static_cast<double>(selUniform), 0.01, 50.0);
					object.scaleX = clamped;
					object.scaleY = clamped;
					_isDirty = true;
					RebuildObjectSprites();
				}
				float selScaleInput[2] = { static_cast<float>(object.scaleX), static_cast<float>(object.scaleY) };
				if (ImGui::DragFloat2("選択中スケール (X, Y)", selScaleInput, 0.05f, 0.05f, 20.0f, "%.2f")) {
					object.scaleX = Clamp(static_cast<double>(selScaleInput[0]), 0.01, 50.0);
					object.scaleY = Clamp(static_cast<double>(selScaleInput[1]), 0.01, 50.0);
					_isDirty = true;
					RebuildObjectSprites();
				}
				if (ImGui::Button("スケールリセット (1.0x)")) {
					object.scaleX = 1.0;
					object.scaleY = 1.0;
					_isDirty = true;
					RebuildObjectSprites();
				}
				ImGui::SameLine();
				if (ImGui::Button("選択オブジェクトを削除")) {
					_objects.erase(_objects.begin() + _selectedObject);
					_selectedObject = -1;
					_isDirty = true;
					RebuildObjectSprites();
				}
			}
			ImGui::SeparatorText("キャラクター");
			if (not _characterCatalog.empty()) {
				std::vector<std::string> names;
				std::vector<const char*> labels;
				for (const auto& info : _characterCatalog) {
				names.push_back(info.displayName + " (" + BoardCharacterTypeKey(info.type) + ")");
				}
				for (const auto& name : names) labels.push_back(name.c_str());
				_selectedCharacter = Clamp(_selectedCharacter, 0, static_cast<int>(labels.size()) - 1);
				if (ImGui::Combo("種類", &_selectedCharacter, labels.data(), static_cast<int>(labels.size()))) {
					_catalogSelection = -1;
				}
				if (_catalogSelection != _selectedCharacter) {
					CopyToBuffer(_characterNameInput, _characterCatalog[_selectedCharacter].displayName);
					CopyToBuffer(_characterIconInput, _characterCatalog[_selectedCharacter].icon);
					_catalogSelection = _selectedCharacter;
				}
				auto& characterInfo = _characterCatalog[_selectedCharacter];
				if (ImGui::InputText("表示名", _characterNameInput.data(), _characterNameInput.size())) {
					characterInfo.displayName = _characterNameInput.data();
					for (auto& placed : _characters) {
						if (placed.type == characterInfo.type) placed.displayName = characterInfo.displayName;
					}
					_isDirty = true;
					RebuildCharacterSprites();
				}
				if (ImGui::InputText("アイコン", _characterIconInput.data(), _characterIconInput.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
					characterInfo.icon = _characterIconInput.data();
					for (auto& placed : _characters) {
						if (placed.type == characterInfo.type) placed.icon = characterInfo.icon;
					}
					_isDirty = true;
					RebuildCharacterSprites();
				}
				ImGui::RadioButton("味方", &_characterTeam, 0);
				ImGui::SameLine();
				ImGui::RadioButton("敵", &_characterTeam, 1);
				ImGui::TextWrapped("同じタイルに配置した敵同士は「グループ」として扱われ、盤面でひとかたまりとして動き、戦闘でもまとめて相手にします。");
				if (ImGui::Button("直前のキャラクターを削除")) {
					if (not _characters.empty()) {
						_characters.pop_back();
						_isDirty = true;
						RebuildCharacterSprites();
					}
				}
				ImGui::TextWrapped("「キャラクター配置」ツールを選択後、タイルをクリックして配置します。同種の味方を再配置すると初期位置が移動します。敵は複数配置可能です。");
			}
			ImGui::Separator();
			ImGui::TextWrapped("右ドラッグ: パン移動 | ホイール: ズーム | タイル配置: 左ドラッグで連続配置");
			if (not _status.empty()) ImGui::TextWrapped("%s", _status.c_str());
		}
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(480.0f, 720.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("アセットエクスプローラー")) {
			ImGui::TextUnformatted("Assets/ (PNGテクスチャ)");
			ImGui::InputText("検索", _assetSearch.data(), _assetSearch.size());
			ImGui::SameLine();
			if (ImGui::Button("更新")) RefreshAssetFiles();
			ImGui::SliderFloat("アイコンサイズ", &_thumbnailSize, 48.0f, 160.0f, "%.0f px");

			const std::string search = _assetSearch.data();
			std::vector<std::string> visibleAssets;
			visibleAssets.reserve(_assetFiles.size());
			for (const auto& assetPath : _assetFiles) {
				if (not search.empty() && assetPath.find(search) == std::string::npos) continue;
				visibleAssets.push_back(assetPath);
			}
			ImGui::TextDisabled("項目数: %zu", visibleAssets.size());

			ImGui::BeginChild("AssetGrid", ImVec2(0.0f, -96.0f), ImGuiChildFlags_Borders);
			const float availWidth = ImGui::GetContentRegionAvail().x;
			const float cardPadding = 6.0f;
			const float cardWidth = _thumbnailSize + cardPadding * 2.0f;
			const float cardHeight = _thumbnailSize + cardPadding * 2.0f + 32.0f;
			const float spacing = ImGui::GetStyle().ItemSpacing.x;
			const int columns = Max(1, static_cast<int>((availWidth + spacing) / (cardWidth + spacing)));

			int currentColumn = 0;
			ImDrawList* const drawList = ImGui::GetWindowDrawList();
			for (const auto& assetPath : visibleAssets) {
				if (0 < currentColumn) {
					ImGui::SameLine();
				}

				ImGui::PushID(assetPath.c_str());
				const ImVec2 startPos = ImGui::GetCursorScreenPos();
				const bool isSelected = (assetPath == _selectedAssetPath);

				const bool clicked = ImGui::InvisibleButton("card", ImVec2(cardWidth, cardHeight));
				const bool isHovered = ImGui::IsItemHovered();
				const bool isDoubleClicked = isHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
				const bool isVisible = ImGui::IsItemVisible();

				const ImVec2 pMin = startPos;
				const ImVec2 pMax(startPos.x + cardWidth, startPos.y + cardHeight);
				const float rounding = 4.0f;

				if (isSelected) {
					drawList->AddRectFilled(pMin, pMax, IM_COL32(50, 110, 190, 100), rounding);
					drawList->AddRect(pMin, pMax, IM_COL32(90, 160, 250, 255), rounding, 0, 1.5f);
				}
				else if (isHovered) {
					drawList->AddRectFilled(pMin, pMax, IM_COL32(255, 255, 255, 30), rounding);
					drawList->AddRect(pMin, pMax, IM_COL32(130, 190, 250, 180), rounding, 0, 1.0f);
				}

				int texW = 0;
				int texH = 0;
				ID3D11ShaderResourceView* srv = nullptr;
				if (isVisible) {
					srv = GetOrCreateThumbnail(assetPath, &texW, &texH);
				}

				const ImVec2 thumbMin(pMin.x + cardPadding, pMin.y + cardPadding);
				const ImVec2 thumbMax(thumbMin.x + _thumbnailSize, thumbMin.y + _thumbnailSize);

				float drawW = _thumbnailSize;
				float drawH = _thumbnailSize;
				if (0 < texW && 0 < texH) {
					const float aspect = static_cast<float>(texW) / static_cast<float>(texH);
					if (aspect <= 1.0f) {
						drawW = _thumbnailSize * aspect;
						drawH = _thumbnailSize;
					}
					else {
						drawW = _thumbnailSize;
						drawH = _thumbnailSize / aspect;
					}
				}
				const float offsetX = (_thumbnailSize - drawW) * 0.5f;
				const float offsetY = (_thumbnailSize - drawH) * 0.5f;
				const ImVec2 imgMin(thumbMin.x + offsetX, thumbMin.y + offsetY);
				const ImVec2 imgMax(imgMin.x + drawW, imgMin.y + drawH);

				drawList->AddRectFilled(thumbMin, thumbMax, IM_COL32(24, 24, 28, 200), 2.0f);
				if (srv != nullptr) {
					drawList->AddImage(reinterpret_cast<ImTextureID>(srv), imgMin, imgMax);
				}
				else if (isVisible) {
					const char* const placeholder = "画像なし";
					const ImVec2 phSize = ImGui::CalcTextSize(placeholder);
					const ImVec2 phPos(thumbMin.x + (_thumbnailSize - phSize.x) * 0.5f, thumbMin.y + (_thumbnailSize - phSize.y) * 0.5f);
					drawList->AddText(phPos, IM_COL32(140, 140, 140, 255), placeholder);
				}
				drawList->AddRect(thumbMin, thumbMax, IM_COL32(60, 60, 70, 150), 2.0f);

				const std::string filename = std::filesystem::path(assetPath).filename().string();
				const ImVec2 textSize = ImGui::CalcTextSize(filename.c_str());
				float textX = pMin.x + cardPadding;
				if (textSize.x < _thumbnailSize) {
					textX += (_thumbnailSize - textSize.x) * 0.5f;
				}
				const float textY = thumbMax.y + 4.0f;
				drawList->PushClipRect(ImVec2(pMin.x + 2.0f, textY), ImVec2(pMax.x - 2.0f, pMax.y - 2.0f), true);
				drawList->AddText(ImVec2(textX, textY), IM_COL32(220, 220, 220, 255), filename.c_str());
				drawList->PopClipRect();

				if (clicked) {
					_selectedAssetPath = assetPath;
				}
				if (isDoubleClicked) {
					_selectedAssetPath = assetPath;
					ApplySelectedAssetToTool();
				}

				if (isHovered) {
					ImGui::BeginTooltip();
					ImGui::Text("%s", filename.c_str());
					ImGui::Separator();
					ImGui::Text("パス: Assets/%s", assetPath.c_str());
					if (0 < texW && 0 < texH) {
						ImGui::Text("サイズ: %d x %d px", texW, texH);
					}
					if (srv != nullptr) {
						const float previewBase = 128.0f;
						float pW = previewBase;
						float pH = previewBase;
						if (0 < texW && 0 < texH) {
							const float pAspect = static_cast<float>(texW) / static_cast<float>(texH);
							if (pAspect <= 1.0f) {
								pW = previewBase * pAspect;
								pH = previewBase;
							}
							else {
								pW = previewBase;
								pH = previewBase / pAspect;
							}
						}
						ImGui::Image(reinterpret_cast<ImTextureID>(srv), ImVec2(pW, pH));
					}
					ImGui::EndTooltip();
				}

				ImGui::PopID();

				++currentColumn;
				if (columns <= currentColumn) {
					currentColumn = 0;
				}
			}
			ImGui::EndChild();

			if (_selectedAssetPath.empty()) {
				ImGui::TextDisabled("PNGテクスチャを選択してください。");
			}
			else {
				int selW = 0;
				int selH = 0;
				ID3D11ShaderResourceView* const selSrv = GetOrCreateThumbnail(_selectedAssetPath, &selW, &selH);
				if (selSrv != nullptr) {
					ImGui::Image(reinterpret_cast<ImTextureID>(selSrv), ImVec2(36.0f, 36.0f));
					ImGui::SameLine();
				}
				ImGui::BeginGroup();
				ImGui::TextUnformatted(_selectedAssetPath.c_str());
				if (0 < selW && 0 < selH) {
					ImGui::TextDisabled("%d x %d px", selW, selH);
				}
				ImGui::EndGroup();

				if (ImGui::Button("タイルブラシに設定")) {
					CopyToBuffer(_tileBrushInput, _selectedAssetPath);
					_status = "タイルブラシのテクスチャを設定しました。";
				}
				ImGui::SameLine();
				if (ImGui::Button("オブジェクトに設定")) {
					CopyToBuffer(_objectTextureInput, _selectedAssetPath);
					_status = "オブジェクトのテクスチャを設定しました。";
				}
			}
		}
		ImGui::End();
	}

	void MapEditor::ClearThumbnailCache() {
		for (auto& [path, info] : _thumbnailCache) {
			if (info.srv != nullptr) {
				info.srv->Release();
				info.srv = nullptr;
			}
		}
		_thumbnailCache.clear();
	}

	ID3D11ShaderResourceView* MapEditor::GetOrCreateThumbnail(const std::string& assetPath, int* outWidth, int* outHeight) {
		const auto it = _thumbnailCache.find(assetPath);
		if (it != _thumbnailCache.end()) {
			if (outWidth != nullptr) *outWidth = it->second.width;
			if (outHeight != nullptr) *outHeight = it->second.height;
			return it->second.srv;
		}

		MapEditorAssetThumbnail info;
		const auto textureAsset = LoadAsset<Texture>(assetPath);
		if (textureAsset) {
			const int handle = textureAsset->GetHandle();
			if (not (handle < 0)) {
				int width = 0;
				int height = 0;
				DxLib::GetGraphSize(handle, &width, &height);
				info.width = width;
				info.height = height;

				ID3D11Device* const device = static_cast<ID3D11Device*>(const_cast<void*>(DxLib::GetUseDirect3D11Device()));
				ID3D11Texture2D* const d3dTexture = static_cast<ID3D11Texture2D*>(const_cast<void*>(DxLib::GetGraphID3D11Texture2D(handle)));
				if (device != nullptr && d3dTexture != nullptr) {
					ID3D11ShaderResourceView* srv = nullptr;
					const HRESULT hr = device->CreateShaderResourceView(d3dTexture, nullptr, &srv);
					if (SUCCEEDED(hr)) {
						info.srv = srv;
					}
				}
			}
		}

		_thumbnailCache[assetPath] = info;
		if (outWidth != nullptr) *outWidth = info.width;
		if (outHeight != nullptr) *outHeight = info.height;
		return info.srv;
	}

	void MapEditor::RefreshAssetFiles() {
		ClearThumbnailCache();
		_assetFiles.clear();
		const auto assetsRoot = AssetManager.ResolvePath("");
		std::error_code error;
		if (not std::filesystem::exists(assetsRoot, error)) return;
		for (auto it = std::filesystem::recursive_directory_iterator(
			assetsRoot, std::filesystem::directory_options::skip_permission_denied, error);
			it != std::filesystem::recursive_directory_iterator(); it.increment(error)) {
			if (error) {
				error.clear();
				continue;
			}
			const bool isRegularFile = it->is_regular_file(error);
			if (error) {
				error.clear();
				continue;
			}
			if (not isRegularFile) continue;
			std::string extension = it->path().extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char value) {
				return static_cast<char>(std::tolower(value));
			});
			if (extension != ".png") continue;
			const auto relative = std::filesystem::relative(it->path(), assetsRoot, error);
			if (not error) _assetFiles.push_back(relative.generic_string());
			error.clear();
		}
		std::sort(_assetFiles.begin(), _assetFiles.end());
		if (std::find(_assetFiles.begin(), _assetFiles.end(), _selectedAssetPath) == _assetFiles.end()) {
			_selectedAssetPath.clear();
		}
	}

	void MapEditor::ApplySelectedAssetToTool() {
		if (_selectedAssetPath.empty()) return;
		if (_tool == MapEditorTool::PaintTile) {
			CopyToBuffer(_tileBrushInput, _selectedAssetPath);
			_status = "タイルブラシのテクスチャを設定しました。";
		}
		else if (_tool == MapEditorTool::PlaceObject) {
			CopyToBuffer(_objectTextureInput, _selectedAssetPath);
			_status = "オブジェクトのテクスチャを設定しました。";
		}
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
		if (not leftDown) {
			_lastPaintX = -1;
			_lastPaintY = -1;
			return;
		}
		const auto world = Camera.ScreenToWorld(screen);
		const int x = static_cast<int>(std::floor((world.x - _boardPosition.x) / _tileSize));
		const int y = static_cast<int>(std::floor((world.y - _boardPosition.y) / _tileSize));
		if (_tool == MapEditorTool::PaintTile) {
			if (not (x < 0) && not (y < 0) && x < _width && y < _height) {
				if (leftPressed || _lastPaintX != x || _lastPaintY != y) {
					bool changed = false;
					if (not (_lastPaintX < 0) && not (_lastPaintY < 0)) {
						changed = PaintLine(_lastPaintX, _lastPaintY, x, y);
					}
					else {
						changed = PaintTileAt(x, y);
					}
					_lastPaintX = x;
					_lastPaintY = y;
					_selectedX = x;
					_selectedY = y;
					if (changed) {
						_isDirty = true;
						RebuildTileSprites();
					}
				}
			}
			return;
		}
		if (not leftPressed) return;
		if (x < 0 || y < 0 || not (x < _width) || not (y < _height)) return;
		if (_tool == MapEditorTool::Select) {
			if (0 <= _occupyEditIndex && _occupyEditIndex < static_cast<int>(_characters.size())) ToggleOccupiedTile(x, y);
			else SelectAt(x, y);
		}
		else PlaceAt(x, y);
	}

	void MapEditor::ToggleOccupiedTile(int x, int y) {
		auto& character = _characters[_occupyEditIndex];
		if (x == character.x && y == character.y) {
			_status = "本体のマスは占有マスにできません。";
			return;
		}
		const auto existing = std::find(character.extraTiles.begin(), character.extraTiles.end(), std::make_pair(x, y));
		if (existing != character.extraTiles.end()) {
			character.extraTiles.erase(existing);
			_isDirty = true;
			RebuildCharacterSprites();
			return;
		}
		// 他のキャラクターが使っているマスは占有マスにできない
		for (int index = 0; index < static_cast<int>(_characters.size()); ++index) {
			if (index == _occupyEditIndex) continue;
			const auto& other = _characters[index];
			if (other.x == x && other.y == y) {
				_status = "そのマスは他のキャラクターが使っています。";
				return;
			}
			if (std::find(other.extraTiles.begin(), other.extraTiles.end(), std::make_pair(x, y)) != other.extraTiles.end()) {
				_status = "そのマスは他のキャラクターが使っています。";
				return;
			}
		}
		character.extraTiles.push_back({ x, y });
		_isDirty = true;
		RebuildCharacterSprites();
	}

	bool MapEditor::PaintTileAt(int x, int y) {
		if (x < 0 || y < 0 || not (x < _width) || not (y < _height)) return false;
		auto& tile = _tiles[y * _width + x];
		const std::string brushTexture = _tileBrushInput.data();
		if (tile.texture == brushTexture && tile.passable == _tilePassable) return false;
		tile.texture = brushTexture;
		tile.passable = _tilePassable;
		return true;
	}

	bool MapEditor::PaintLine(int x0, int y0, int x1, int y1) {
		bool anyChanged = false;
		const int dx = std::abs(x1 - x0);
		const int dy = std::abs(y1 - y0);
		const int sx = x0 < x1 ? 1 : -1;
		const int sy = y0 < y1 ? 1 : -1;
		int err = dx - dy;
		int curX = x0;
		int curY = y0;
		while (true) {
			if (PaintTileAt(curX, curY)) anyChanged = true;
			if (curX == x1 && curY == y1) break;
			const int e2 = 2 * err;
			if (not (e2 < -dy)) {
				err -= dy;
				curX += sx;
			}
			if (e2 < dx || e2 == dx) {
				err += dx;
				curY += sy;
			}
		}
		return anyChanged;
	}

	void MapEditor::PlaceAt(int x, int y) {
		if (_tool == MapEditorTool::PaintTile) {
			if (PaintTileAt(x, y)) {
				_selectedX = x;
				_selectedY = y;
				_isDirty = true;
				RebuildTileSprites();
			}
			return;
		}
		if (_tool == MapEditorTool::PlaceObject) {
			if (_objectTextureInput[0] == '\0') {
				_status = "配置するオブジェクトのテクスチャを指定してください。";
				return;
			}
			const double sX = _objectScaleUniform ? static_cast<double>(_objectScale) : static_cast<double>(_objectScaleX);
			const double sY = _objectScaleUniform ? static_cast<double>(_objectScale) : static_cast<double>(_objectScaleY);
			_objects.push_back({
				_objectTextureInput.data(),
				x, y,
				1, 1,
				Clamp(sX, 0.01, 50.0),
				Clamp(sY, 0.01, 50.0)
			});
			_selectedObject = static_cast<int>(_objects.size()) - 1;
			_isDirty = true;
			RebuildObjectSprites();
			_status = "オブジェクトを配置しました。";
			return;
		}
		if (_tool == MapEditorTool::PlaceCharacter && not (_selectedCharacter < 0)
			&& _selectedCharacter < static_cast<int>(_characterCatalog.size())) {
			const auto& info = _characterCatalog[_selectedCharacter];
			const auto team = _characterTeam == 0 ? BoardTeam::Ally : BoardTeam::Enemy;
			std::string id = team == BoardTeam::Ally
				? std::string("ally_") + BoardCharacterTypeKey(info.type) : std::string();
			std::string moveType = "adjacent";
			std::vector<std::pair<int, int>> extraTiles;
			double iconOffsetX = 0.0;
			double iconOffsetY = 0.0;
			double iconScale = 1.0;
			if (team == BoardTeam::Ally) {
				const auto existing = std::find_if(_characters.begin(), _characters.end(), [&info](const auto& character) {
					return character.team == BoardTeam::Ally && character.type == info.type;
				});
				if (existing != _characters.end()) {
					id = existing->id;
					moveType = existing->moveType;
					iconOffsetX = existing->iconOffsetX;
					iconOffsetY = existing->iconOffsetY;
					iconScale = existing->iconScale;
					// 再配置では占有マスも同じだけずらして引き継ぐ
					for (const auto& extra : existing->extraTiles) {
						extraTiles.push_back({ extra.first + (x - existing->x), extra.second + (y - existing->y) });
					}
				}
				std::erase_if(_characters, [&info](const auto& character) {
					return character.team == BoardTeam::Ally && character.type == info.type;
				});
			}
			else {
				int index = 1;
				do {
					id = "enemy_" + std::to_string(index++);
				} while (std::any_of(_characters.begin(), _characters.end(), [&id](const auto& character) {
					return character.id == id;
				}));
			}
			MapEditorCharacterData character;
			character.id = id;
			character.type = info.type;
			character.displayName = info.displayName;
			character.icon = info.icon;
			character.team = team;
			character.moveType = moveType;
			character.x = x;
			character.y = y;
			character.extraTiles = extraTiles;
			character.iconOffsetX = iconOffsetX;
			character.iconOffsetY = iconOffsetY;
			character.iconScale = iconScale;
			_characters.push_back(character);
			_isDirty = true;
			RebuildCharacterSprites();
			_status = team == BoardTeam::Ally
				? "味方の初期位置を設定しました。"
				: "敵を配置しました。";
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
			double objWidth = static_cast<double>(_tileSize);
			double objHeight = static_cast<double>(_tileSize);
			if (index < static_cast<int>(_objectSprites.size()) && _objectSprites[index]) {
				const auto rawSize = _objectSprites[index]->GetRawSize();
				if (0.0 < rawSize.x && 0.0 < rawSize.y) {
					objWidth = rawSize.x * object.scaleX;
					objHeight = rawSize.y * object.scaleY;
				}
			}
			if (not (world.x < left) && world.x < left + objWidth
				&& not (world.y < top) && world.y < top + objHeight) {
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

	Vector2D MapEditor::TilePosition(int x, int y) const {
		return _boardPosition + Vector2D(x * _tileSize, y * _tileSize);
	}
}
