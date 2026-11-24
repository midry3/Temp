#include "CoopLine.h"
#include <Graphic/2D.h>
#include <DxLib.h>

namespace Game {
	void CoopLine::Setup(const SharedPtr<BoardUnit>& a, const SharedPtr<BoardUnit>& b, const SharedPtr<BoardUnit>& extendFrom, const CenterResolver& centerResolver) {
		_a = a;
		_b = b;
		_extendFrom = extendFrom;
		_centerResolver = centerResolver;
		_progress = extendFrom ? 0.0 : 1.0;
	}

	void CoopLine::_Update(float dt) {
		if (_progress < 1.0) {
			_progress += dt / EXTEND_SECONDS;
			if (1.0 < _progress) _progress = 1.0;
		}
	}

	void CoopLine::_Draw() const {
		if (not _a || not _b || not _centerResolver) return;
		const auto& fromUnit = _extendFrom ? _extendFrom : _a;
		const auto& toUnit = fromUnit == _a ? _b : _a;
		const auto from = _centerResolver(fromUnit);
		const auto to = _centerResolver(toUnit);
		const auto current = from + (to - from) * _progress;
		NovotRenderer::DrawLine(from, current, GetColor(COLOR_R, COLOR_G, COLOR_B), THICKNESS);
	}
}
