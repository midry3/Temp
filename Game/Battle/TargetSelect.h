#pragma once
#include <Scene.h>
#include <Scene/Button/Button.h>
#include <Scene/Shape/ColorRect.h>
#include <functional>
#include <utility>
#include <vector>
#include "BattleCharacter/BattleCharacter.h"

namespace Game {
	// 単体対象スキルの対象選択。候補パネルの矩形以外を薄く暗くし、
	// パネル位置に置いたボタンへのクリックで対象を選ぶ。
	class TargetSelect : public Scene {
	private:
		std::vector<SharedPtr<ColorRect>> _dims;
		std::vector<SharedPtr<Button>> _buttons;
		std::function<void(SharedPtr<BattleCharacter>)> _onSelected;

		void Select(const SharedPtr<BattleCharacter>& character);
	public:
		// candidates は (対象キャラ, パネルの左上座標と大きさ) の一覧
		void Show(const std::vector<std::pair<SharedPtr<BattleCharacter>, std::pair<Vector2D, Vector2D>>>& candidates,
			const std::function<void(SharedPtr<BattleCharacter>)>& onSelected);
		void Hide();
	};
}
