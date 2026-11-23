#include "BoardWinScreen.h"
#include <Scene/Shape/ColorRect.h>
#include <Scene/Text/TextLabel.h>
#include <Scene/Button/Button.h>

namespace Game {
	void BoardWinScreen::_Load() {
		SetUIMode(true);

		const auto wsize = PropertyManager.GetSettingWindowSize();
		const auto& button = CreateScene<Button>();
		button->SetArea({ {0, 0}, {wsize.x, 0}, {wsize.x, wsize.y}, {0, wsize.y} });
		button->LeftPressed += [this](const MouseEvent& e) {
			if (e.just && _isButtonEnabled) {
				// 盤面側では遷移先が未定のため、シーン遷移は行わない
				// SceneManager.TransSceneTo(CommonData::CurrentBoard, 1.0, 0.3);
			}
			};

		const auto& background = CreateScene<ColorRect>();
		background->SetSize(wsize)
			->SetColor({ 0, 0, 0 });
		const auto& label = CreateScene<TextLabel>();
		label->BeginSetting()
			->SetFontSize(150)
			->SetForegroundColor({ 255, 255, 255 })
			->EndSetting()
			->SetText("You Win!")
			->SetPosition({ 300, 300 });

		CreateTweenGroup()->CreateTween(&background->alpha, 150, 0.5)->From(0);
		CreateTweenGroup()->CreateTween(&label->offset.x, 0, 1.0, Easing::EaseInCubic)->From(1500);

		CreateTimer(WAIT_TIME, [this]() {
			_isButtonEnabled = true;
			});
	}
}
