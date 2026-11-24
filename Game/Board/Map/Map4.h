#pragma once
#include "../BoardBase.h"

namespace Game {
	class Map4 : public BoardBase {
	protected:
		void _Load() override {
			BoardBase::_Load();
			LoadBoard("board/map4.lua");
		}
	};
}