#include "StageSelect.h"
#include "Board/Map/StageBoard.h"
#include <Scene/Button/Button.h>
#include <Scene/Shape/ColorPolygon.h>
#include <Scene/Text/TextLabel.h>
#include <Scene/Sound/SoundPlayer.h>
#include <Game/Data/SoundEffects.h>
#include <Graphic/2D.h>
#include <DxLib.h>
#include <cmath>
#include <cstdio>

namespace Game {
	Vector2D StageSelect::NodePosition(int index) const {
		// 横に等間隔で並べ、偶数番は上、奇数番は下にしてジグザグにする
		const double y = NODE_CENTER_Y + (index % 2 == 0 ? -NODE_ZIGZAG_Y : NODE_ZIGZAG_Y);
		return { NODE_START_X + NODE_SPACING_X * index, y };
	}

	void StageSelect::_Load() {
		const unsigned int nodeColor = CoreBackend::Drawing::Color(NODE_COLOR_R, NODE_COLOR_G, NODE_COLOR_B);
		const unsigned int nodeHoverColor = CoreBackend::Drawing::Color(NODE_HOVER_COLOR_R, NODE_HOVER_COLOR_G, NODE_HOVER_COLOR_B);

		for (int index = 0; index < STAGE_COUNT; ++index) {
			const int stageNumber = index + 1;
			const auto center = NodePosition(index);

			// 頂点のクリック判定（円を多角形で近似する）
			const auto& button = CreateScene<Button>();
			button->SetPosition(center);
			std::vector<Vector2D> circle;
			for (int s = 0; s < NODE_SEGMENTS; ++s) {
				const double angle = 2.0 * 3.14159265358979323846 * s / NODE_SEGMENTS;
				circle.push_back({ std::cos(angle) * NODE_RADIUS, std::sin(angle) * NODE_RADIUS });
			}
			button->SetArea(circle);

			// 頂点の丸
			const auto& disc = button->CreateScene<ColorPolygon>();
			disc->SetVertices(circle);
			disc->SetColor(nodeColor);

			button->Entered += [disc, nodeHoverColor](const MouseEvent& event) {
				disc->SetColor(nodeHoverColor);
				};
			button->Exited += [disc, nodeColor](const MouseEvent& event) {
				disc->SetColor(nodeColor);
				};
			const auto& clickSE = CreateSelectSePlayer(button.get());
			button->LeftPressed += [this, stageNumber, clickSE](const MouseEvent& event) {
				if (not event.just) return;
				clickSE->Play();
				char file[64];
				std::snprintf(file, sizeof(file), "board/map%d.lua", stageNumber);
				const auto& board = MakeShared<StageBoard>(file);
				SceneManager.TransSceneTo(board, 0.3, 0.3, true);
				};

			// 頂点の上に「ステージn」ラベル
			const auto& label = CreateScene<TextLabel>();
			label->BeginSetting()
				->SetFontSize(LABEL_FONT_SIZE)
				->SetForegroundColor({ LABEL_COLOR_R, LABEL_COLOR_G, LABEL_COLOR_B })
				->EndSetting()
				->SetText("ステージ" + std::to_string(stageNumber))
				->SetWidth(static_cast<int>(LABEL_WIDTH));
			label->SetPosition({ center.x - LABEL_WIDTH / 2.0, center.y - NODE_RADIUS - LABEL_GAP_Y - LABEL_FONT_SIZE });
		}
	}

	void StageSelect::_Draw() const {
		// 背景と辺は子シーン（頂点・ラベル）より先に描く
		// （このシーンは原点にあるため、そのまま画面座標になる）
		const auto windowSize = PropertyManager.GetSettingWindowSize();
		DrawBox(0, 0, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y),
			GetColor(BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B), TRUE);
		const unsigned int lineColor = CoreBackend::Drawing::Color(LINE_COLOR_R, LINE_COLOR_G, LINE_COLOR_B);
		for (int index = 0; index + 1 < STAGE_COUNT; ++index) {
			NovotRenderer::DrawLine(NodePosition(index), NodePosition(index + 1), lineColor, LINE_THICKNESS);
		}
	}
}
