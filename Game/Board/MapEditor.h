#pragma once
#include <Scene.h>
#include <Scene/Texture/Sprite2D.h>
#include <Scene/Text/TextLabel.h>
#include <array>
#include <string>
#include <vector>
#include "BoardTypes.h"

namespace Game {
	enum class MapEditorTool {
		Select,
		PaintTile,
		PlaceObject,
		PlaceCharacter
	};

	struct MapEditorTileData {
		std::string texture;
		bool passable = true;
	};

	struct MapEditorObjectData {
		std::string texture;
		int x = 0;
		int y = 0;
		int width = 1;
		int height = 1;
	};

	struct MapEditorCharacterData {
		std::string id;
		BoardCharacterType type = BoardCharacterType::Plea;
		std::string displayName;
		std::string icon;
		BoardTeam team = BoardTeam::Ally;
		std::string moveType = "adjacent";
		int x = 0;
		int y = 0;
	};

	struct MapEditorCharacterInfo {
		BoardCharacterType type = BoardCharacterType::Plea;
		std::string displayName;
		std::string icon;
	};

	class MapEditor : public Scene {
	private:
		int _width = 9;
		int _height = 7;
		int _tileSize = 72;
		Vector2D _boardPosition{ 320.0, 48.0 };
		std::vector<MapEditorTileData> _tiles;
		std::vector<SharedPtr<Sprite2D>> _tileSprites;
		std::vector<MapEditorObjectData> _objects;
		std::vector<SharedPtr<Sprite2D>> _objectSprites;
		std::vector<MapEditorCharacterData> _characters;
		std::vector<SharedPtr<Sprite2D>> _characterSprites;
		std::vector<SharedPtr<TextLabel>> _characterLabels;
		std::vector<MapEditorCharacterInfo> _characterCatalog;
		std::string _mapPath = "board/default.lua";
		std::string _defaultTileTexture;
		std::string _tileTexture;
		bool _tilePassable = true;
		int _objectWidth = 1;
		int _objectHeight = 1;
		int _selectedX = -1;
		int _selectedY = -1;
		int _selectedCharacter = 0;
		int _selectedObject = -1;
		MapEditorTool _tool = MapEditorTool::Select;
		Vector2D _lastMouseScreen;
		bool _isPanning = false;
		bool _isDirty = false;
		std::string _status;
		std::array<char, 512> _mapPathInput{};
		std::array<char, 512> _tileTextureInput{};
		std::array<char, 512> _tileBrushInput{};
		std::array<char, 512> _defaultTextureInput{};
		std::array<char, 512> _objectTextureInput{};
		std::array<char, 128> _characterNameInput{};
		std::array<char, 512> _characterIconInput{};
		int _characterTeam = 0;
		int _catalogSelection = -1;
		bool _leftMouseDown = false;

		bool LoadMap(const std::string& path);
		bool SaveMap();
		void ResizeMap(int width, int height);
		void RebuildTileSprites();
		void RebuildObjectSprites();
		void RebuildCharacterSprites();
		void UpdateEditorUI();
		void HandleViewportInput();
		void PlaceAt(int x, int y);
		void SelectAt(int x, int y);
		Vector2D TilePosition(int x, int y) const;

	protected:
		void _Load() override;
		void _Release() override;
		void _Update(float dt) override;
		void _Draw() const override;
	};
}
