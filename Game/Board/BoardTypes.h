#pragma once
#include <string>

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
		Slime
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
	};
}
