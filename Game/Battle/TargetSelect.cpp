#include "TargetSelect.h"
#include <Game/Data/SoundEffects.h>
#include <cmath>

namespace Game {
	void TargetSelect::Show(const std::vector<std::pair<SharedPtr<BattleCharacter>, std::pair<Vector2D, Vector2D>>>& candidates,
		const std::function<void(SharedPtr<BattleCharacter>)>& onSelected) {
		Hide();
		_onSelected = onSelected;
		if (candidates.empty()) return;

		Vector2D window = PropertyManager.GetSettingWindowSize();
		if (not (0.0 < window.x) || not (0.0 < window.y)) {
			window = PropertyManager.GetRealWindowSize();
		}

		// 候補パネル全体の外接矩形を求め、その外側だけを薄く暗くする
		double left = candidates.front().second.first.x;
		double top = candidates.front().second.first.y;
		double right = left + candidates.front().second.second.x;
		double bottom = top + candidates.front().second.second.y;
		for (const auto& [character, rect] : candidates) {
			left = Min(left, rect.first.x);
			top = Min(top, rect.first.y);
			right = Max(right, rect.first.x + rect.second.x);
			bottom = Max(bottom, rect.first.y + rect.second.y);
		}

		auto addDim = [this](const Vector2D& position, const Vector2D& size) {
			if (size.x <= 0.0 || size.y <= 0.0) return;
			const auto& dim = CreateScene<ColorRect>();
			dim->SetSize(size)
				->SetColor({ 0, 0, 0 })
				->SetAlpha(128)
				->SetPosition(position);
			_dims.push_back(dim);
			};
		addDim({ 0.0, 0.0 }, { left, window.y });
		addDim({ right, 0.0 }, { window.x - right, window.y });
		addDim({ left, 0.0 }, { right - left, top });
		addDim({ left, bottom }, { right - left, window.y - bottom });

		// 候補パネルをハイライト色で縁取りし、パネル全体を発光点滅させる
		const CoreBackend::Drawing::Color highlightColor(HighlightColorR, HighlightColorG, HighlightColorB);
		for (const auto& [character, rect] : candidates) {
			const auto& glow = CreateScene<ColorRect>();
			glow->SetSize(rect.second)
				->SetColor(highlightColor)
				->SetAlpha(static_cast<int>(HighlightAlphaMin))
				->SetPosition(rect.first);
			_glows.push_back(glow);

			const double w = HighlightBorderWidth;
			auto addBorder = [this, &highlightColor](const Vector2D& position, const Vector2D& size) {
				const auto& border = CreateScene<ColorRect>();
				border->SetSize(size)
					->SetColor(highlightColor)
					->SetPosition(position);
				_borders.push_back(border);
				};
			addBorder({ rect.first.x - w, rect.first.y - w }, { rect.second.x + w * 2.0, w });
			addBorder({ rect.first.x - w, rect.first.y + rect.second.y }, { rect.second.x + w * 2.0, w });
			addBorder({ rect.first.x - w, rect.first.y }, { w, rect.second.y });
			addBorder({ rect.first.x + rect.second.x, rect.first.y }, { w, rect.second.y });
		}

		for (const auto& [character, rect] : candidates) {
			const auto& button = CreateScene<Button>();
			button->SetArea({
				Vector2D(0, 0),
				Vector2D(rect.second.x, 0),
				rect.second,
				Vector2D(0, rect.second.y)
				});
			button->SetPosition(rect.first);
			const auto& selectSE = CreateSelectSePlayer(button.get());
			button->LeftPressed += [this, selectSE, character](const MouseEvent& event) {
				if (not event.just) return;
				selectSE->Play();
				Select(character);
				};
			_buttons.push_back(button);
		}
	}

	void TargetSelect::_Update(float dt) {
		if (_glows.empty()) return;
		_blinkTime += dt;
		const double phase = (std::sin(_blinkTime * HighlightBlinkSpeed) + 1.0) * 0.5;
		const int alpha = static_cast<int>(HighlightAlphaMin + (HighlightAlphaMax - HighlightAlphaMin) * phase);
		for (const auto& glow : _glows) {
			glow->SetAlpha(alpha);
		}
	}

	void TargetSelect::Select(const SharedPtr<BattleCharacter>& character) {
		const auto onSelected = _onSelected;
		Hide();
		if (onSelected) onSelected(character);
	}

	void TargetSelect::Hide() {
		for (const auto& dim : _dims) {
			DeleteScene(dim);
		}
		_dims.clear();
		for (const auto& glow : _glows) {
			DeleteScene(glow);
		}
		_glows.clear();
		for (const auto& border : _borders) {
			DeleteScene(border);
		}
		_borders.clear();
		for (const auto& button : _buttons) {
			DeleteScene(button);
		}
		_buttons.clear();
		_onSelected = nullptr;
		_blinkTime = 0.0;
	}
}
