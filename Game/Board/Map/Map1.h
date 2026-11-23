#pragma once
#include "../BoardBase.h"

namespace Game {
	class Map1 : public BoardBase {
	protected:
		void _Load() override {
			BoardBase::_Load();
			LoadBoard("board/map1.lua");
		}
	};
}