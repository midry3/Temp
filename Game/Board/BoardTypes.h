#pragma once
#include <string>
#include <vector>

namespace Game {
	enum class BoardTerrain {
		Normal,
		Disabled,
		Other
	};

	enum class BoardTeam {
		Ally,
		Enemy
	};

	enum class BoardMoveType {
		DiagonalUnlimited,
		Adjacent,
		RangeTwo,
		OrthogonalUnlimited
	};

	enum class BoardCharacterType {
		Plea,
		Reko,
		Zon,
		Gou,
		Slime,
		Witch,
		Golem,
		RareSlime,
		RareWitch,
		RareGolem
	};

	struct BoardOffset {
		int dx = 0;
		int dy = 0;
		bool operator==(const BoardOffset& other) const = default;
	};

	struct BoardTileData {
		BoardTerrain terrain = BoardTerrain::Normal;
		std::string texture;
	};

	struct BoardObjectData {
		std::string texture;
		int x = 0;
		int y = 0;
		int width = 1;
		int height = 1;
		double scaleX = 1.0;
		double scaleY = 1.0;
	};

	struct BoardUnitData {
		std::string id;
		std::string displayName;
		BoardCharacterType characterType = BoardCharacterType::Plea;
		BoardTeam team = BoardTeam::Ally;
		BoardMoveType moveType = BoardMoveType::Adjacent;
		std::string texture;
		int x = 0;
		int y = 0;
		// 初期配置タイル。初期配置が同じタイルの敵同士は同じグループとして扱う
		int initialX = -1;
		int initialY = -1;
		// 本体のマス以外に占有するマス（本体からの相対オフセット）。移動に合わせて一緒に動く
		std::vector<BoardOffset> extraTiles;
		// キャラクターアイコンの表示位置オフセット（ピクセル）
		double iconOffsetX = 0.0;
		double iconOffsetY = 0.0;
	};
}
