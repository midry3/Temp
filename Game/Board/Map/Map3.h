#pragma once
#include "../BoardBase.h"

namespace Game {
	class Map3 : public BoardBase {
	protected:
		void _Load() override {
			BoardBase::_Load();
			LoadBoard("board/map3.lua");
		}
	};
}