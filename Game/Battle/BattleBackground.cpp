#include "BattleBackground.h"
#include <DxLib.h>
#include <PropertyManager.h>
#include <cmath>

namespace Game {
	namespace {
		struct Pattern {
			float x;
			float y;
			float radius;
			float speed;
			unsigned int color;
		};

		const Pattern Patterns[] = {
			{ 0.14f, 0.28f, 260.0f, 0.08f, GetColor(105, 45, 50) },
			{ 0.39f, 0.68f, 210.0f, -0.06f, GetColor(92, 70, 38) },
			{ 0.68f, 0.30f, 300.0f, 0.05f, GetColor(74, 50, 42) },
			{ 0.91f, 0.72f, 240.0f, -0.07f, GetColor(88, 82, 42) },
			{ 0.52f, 0.12f, 170.0f, 0.04f, GetColor(90, 52, 48) }
		};

	}

	void BattleBackground::_Update(float dt) {
		_time += dt;
	}

	void BattleBackground::_Draw() const {
		const auto windowSize = PropertyManager.GetSettingWindowSize();
		const int width = static_cast<int>(windowSize.x);
		const int height = static_cast<int>(windowSize.y);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		DrawBox(0, 0, width, height, GetColor(30, 25, 30), TRUE);

		for (const auto& pattern : Patterns) {
			const double phase = _time * pattern.speed;
			const int x = static_cast<int>(
				width * pattern.x + std::cos(phase) * width * 0.07);
			const int y = static_cast<int>(
				height * pattern.y + std::sin(phase * 0.8) * height * 0.06);
			const int radius = static_cast<int>(
				pattern.radius * (1.0 + std::sin(phase * 0.7) * 0.08));

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 16);
			DrawCircle(x, y, radius, pattern.color, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 24);
			DrawCircle(x, y, radius, pattern.color, FALSE);
		}

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 18);
		for (int index = 0; index < 7; ++index) {
			const double phase = _time * 0.08 + index * 0.9;
			const int y = static_cast<int>(
				height * (0.18 + index * 0.11) + std::sin(phase) * 18.0);
			DrawLine(0, y, width, y + static_cast<int>(std::sin(phase * 0.7) * 28.0),
				GetColor(118, 76, 52), 2);
		}
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	}
}
