#pragma once
#include <Game/Board/BoardBase.h>

namespace Game {
	class CommonData {
	public:
		static SharedPtr<BoardBase> CurrentBoard;
	};
}