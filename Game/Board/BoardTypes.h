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

	struct BoardUnitData {
		std::string id;
		BoardTeam team = BoardTeam::Ally;
		BoardMoveType moveType = BoardMoveType::Adjacent;
		std::string texture;
		int x = 0;
		int y = 0;
	};
}
