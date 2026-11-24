#pragma once
#include <Scene.h>
#include <Scene/Button/Button.h>
#include <Scene/Shape/ColorRect.h>
#include <Scene/Text/TextLabel.h>
#include <Scene/Texture/Sprite2D.h>
#include <string>
#include <vector>

namespace Game {
	// ステージ開始時の会話ダイアログ。
	// 左にプレアのアイコンを表示し、左クリックで1行ずつ進み、最後の行で終了する。
	class StageDialogue : public Scene {
	private:
		// ダイアログ枠とアイコンの配置
		static constexpr double BOX_X = 40.0;
		static constexpr double BOX_Y = 540.0;
		static constexpr double BOX_WIDTH = 1200.0;
		static constexpr double BOX_HEIGHT = 150.0;
		static constexpr double ICON_SCALE = 2.0;
		static constexpr double ICON_MARGIN = 20.0;
		static constexpr int TEXT_FONT_SIZE = 20;

		std::vector<std::string> _lines;
		int _lineIndex = 0;
		SharedPtr<TextLabel> _textLabel;

		void ShowCurrentLine();
		void Advance();
	protected:
		void _Load() override;
	public:
		Signal<> Finished;
		void Setup(const std::vector<std::string>& lines);
	};
}
