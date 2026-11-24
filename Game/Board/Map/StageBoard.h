#pragma once
#include "../BoardBase.h"
#include <string>
#include <utility>

namespace Game {
	// ステージ選択から使う汎用ボード。ステージ固有の処理はなく、
	// 指定された盤面 Lua を読み込むだけ。
	class StageBoard : public BoardBase {
	private:
		std::string _boardFile;
	public:
		explicit StageBoard(std::string boardFile) : _boardFile(std::move(boardFile)) {}
	protected:
		void _Load() override {
			BoardBase::_Load();
			LoadBoard(_boardFile);
		}
	};
}
