#pragma once
#include <Scene.h>
#include <Scene/Texture/Sprite2D.h>
#include <Scene/Text/TextLabel.h>
#include <array>
#include <map>
#include <string>
#include <vector>
#include "BoardTypes.h"

struct ID3D11ShaderResourceView;

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
		double scaleX = 1.0;
		double scaleY = 1.0;
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
		// 本体以外に占有するマス（絶対座標）。保存時は本体からの相対オフセットになる
		std::vector<std::pair<int, int>> extraTiles;
		// キャラクターアイコンの表示位置オフセット（ピクセル）
		double iconOffsetX = 0.0;
		double iconOffsetY = 0.0;
		// キャラクターアイコンの表示スケール（1.0が通常サイズ）
		double iconScale = 1.0;
	};

	struct MapEditorCharacterInfo {
		BoardCharacterType type = BoardCharacterType::Plea;
		std::string displayName;
		std::string icon;
	};

	struct MapEditorAssetThumbnail {
		ID3D11ShaderResourceView* srv = nullptr;
		int width = 0;
		int height = 0;
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
		// ステージ開始時の会話（編集はせず、読み込んで保存時にそのまま書き戻す）
		std::vector<std::string> _dialogueLines;
		std::vector<SharedPtr<Sprite2D>> _characterSprites;
		std::vector<SharedPtr<TextLabel>> _characterLabels;
		std::vector<SharedPtr<Sprite2D>> _extraTileSprites;
		int _occupyEditIndex = -1;
		std::vector<MapEditorCharacterInfo> _characterCatalog;
		std::vector<std::string> _assetFiles;
		std::string _mapPath = "board/default.lua";
		std::string _defaultTileTexture;
		std::string _tileTexture;
		std::string _selectedAssetPath;
		bool _tilePassable = true;
		int _objectWidth = 1;
		int _objectHeight = 1;
		float _objectScale = 1.0f;
		float _objectScaleX = 1.0f;
		float _objectScaleY = 1.0f;
		bool _objectScaleUniform = true;
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
		std::array<char, 128> _assetSearch{};
		std::map<std::string, MapEditorAssetThumbnail> _thumbnailCache;
		float _thumbnailSize = 96.0f;
		int _characterTeam = 0;
		int _catalogSelection = -1;
		bool _leftMouseDown = false;
		int _lastPaintX = -1;
		int _lastPaintY = -1;

		bool LoadMap(const std::string& path);
		bool SaveMap();
		void SaveLastOpenedMapPath(const std::string& path);
		[[nodiscard]] std::string LoadLastOpenedMapPath() const;
		void ResizeMap(int width, int height);
		void RebuildTileSprites();
		void RebuildObjectSprites();
		void RebuildCharacterSprites();
		void UpdateEditorUI();
		void RefreshAssetFiles();
		void ClearThumbnailCache();
		ID3D11ShaderResourceView* GetOrCreateThumbnail(const std::string& assetPath, int* outWidth, int* outHeight);
		void ApplySelectedAssetToTool();
		void HandleViewportInput();
		bool PaintTileAt(int x, int y);
		bool PaintLine(int x0, int y0, int x1, int y1);
		void PlaceAt(int x, int y);
		void SelectAt(int x, int y);
		void ToggleOccupiedTile(int x, int y);
		Vector2D TilePosition(int x, int y) const;

	protected:
		void _Load() override;
		void _Release() override;
		void _Update(float dt) override;
		void _Draw() const override;
	};
}
