#pragma once
#include <Scene.h>
#include <functional>
#include "BoardUnit.h"

namespace Game {
	// 共闘可能な味方同士を結ぶライン。両端は各ユニットのタイル中心。
	class CoopLine : public Scene {
	public:
		// ラインの見た目（調整用）
		static constexpr int COLOR_R = 255;
		static constexpr int COLOR_G = 200;
		static constexpr int COLOR_B = 80;
		static constexpr int THICKNESS = 3;
		// 線が伸びるアニメーションの時間（秒）
		static constexpr double EXTEND_SECONDS = 0.35;

		using CenterResolver = std::function<Vector2D(const SharedPtr<BoardUnit>&)>;

		// extendFrom が指定された場合、そのユニット側から線が伸びる。
		// nullptr の場合はアニメーションなしで即座に結ぶ
		void Setup(const SharedPtr<BoardUnit>& a, const SharedPtr<BoardUnit>& b, const SharedPtr<BoardUnit>& extendFrom, const CenterResolver& centerResolver);

	protected:
		void _Update(float dt) override;
		void _Draw() const override;

	private:
		SharedPtr<BoardUnit> _a;
		SharedPtr<BoardUnit> _b;
		SharedPtr<BoardUnit> _extendFrom;
		CenterResolver _centerResolver;
		double _progress = 1.0;
	};
}
