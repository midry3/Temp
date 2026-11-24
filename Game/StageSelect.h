#pragma once
#include <Scene.h>

namespace Game {
	// ステージ選択シーン。グラフ理論のグラフのように、頂点（丸）をステージとして並べ、
	// 頂点をクリックするとそのステージの盤面へ遷移する。
	// 見た目に関する値はすべて下の定数に分離してあるので、デザイン変更はここを触る。
	class StageSelect : public Scene {
	private:
		// ==== ステージ構成 ====
		static constexpr int STAGE_COUNT = 6;

		// ==== 頂点（丸）のデザイン ====
		static constexpr double NODE_RADIUS = 42.0;
		static constexpr int NODE_SEGMENTS = 28;
		static constexpr int NODE_COLOR_R = 70;
		static constexpr int NODE_COLOR_G = 120;
		static constexpr int NODE_COLOR_B = 190;
		static constexpr int NODE_HOVER_COLOR_R = 110;
		static constexpr int NODE_HOVER_COLOR_G = 160;
		static constexpr int NODE_HOVER_COLOR_B = 230;

		// ==== 辺（線）のデザイン ====
		static constexpr int LINE_COLOR_R = 120;
		static constexpr int LINE_COLOR_G = 140;
		static constexpr int LINE_COLOR_B = 180;
		static constexpr int LINE_THICKNESS = 4;

		// ==== 配置（横に並べて上下にジグザグさせる） ====
		static constexpr double NODE_START_X = 140.0;
		static constexpr double NODE_SPACING_X = 200.0;
		static constexpr double NODE_CENTER_Y = 360.0;
		static constexpr double NODE_ZIGZAG_Y = 110.0;

		// ==== ラベル（ステージn）のデザイン ====
		static constexpr int LABEL_FONT_SIZE = 24;
		static constexpr double LABEL_WIDTH = 160.0;
		static constexpr double LABEL_GAP_Y = 12.0;
		static constexpr int LABEL_COLOR_R = 255;
		static constexpr int LABEL_COLOR_G = 255;
		static constexpr int LABEL_COLOR_B = 255;

		// ==== 背景 ====
		static constexpr int BACKGROUND_COLOR_R = 16;
		static constexpr int BACKGROUND_COLOR_G = 20;
		static constexpr int BACKGROUND_COLOR_B = 28;

		[[nodiscard]] Vector2D NodePosition(int index) const;

	protected:
		void _Load() override;
		void _Draw() const override;
	};
}
