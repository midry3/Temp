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
		// ハイライトの調整値（候補パネルをオレンジ寄りの白で縁取りし、発光点滅させる）
		static constexpr int HighlightColorR = 255;
		static constexpr int HighlightColorG = 205;
		static constexpr int HighlightColorB = 150;
		static constexpr double HighlightAlphaMin = 40.0;
		static constexpr double HighlightAlphaMax = 120.0;
		static constexpr double HighlightBlinkSpeed = 6.0;
		static constexpr double HighlightBorderWidth = 3.0;

		std::vector<SharedPtr<ColorRect>> _dims;
		std::vector<SharedPtr<ColorRect>> _glows;
		std::vector<SharedPtr<ColorRect>> _borders;
		std::vector<SharedPtr<Button>> _buttons;
		std::function<void(SharedPtr<BattleCharacter>)> _onSelected;
		double _blinkTime = 0.0;

		void Select(const SharedPtr<BattleCharacter>& character);
	protected:
		void _Update(float dt) override;
	public:
		// candidates は (対象キャラ, パネルの左上座標と大きさ) の一覧
		void Show(const std::vector<std::pair<SharedPtr<BattleCharacter>, std::pair<Vector2D, Vector2D>>>& candidates,
			const std::function<void(SharedPtr<BattleCharacter>)>& onSelected);
		void Hide();
	};
}
