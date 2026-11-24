#include "WinScreen.h"
#include <Scene/Shape/ColorRect.h>
#include <Scene/Text/TextLabel.h>
#include <Scene/Button/Button.h>
#include <Game/Data/CommonData.h>
#include <Scene/Sound/SoundPlayer.h>

namespace Game {
	void WinScreen::_Load() {
		const auto wsize = PropertyManager.GetSettingWindowSize();
		const auto& button = CreateScene<Button>();
		button->SetArea({ {0, 0}, {wsize.x, 0}, {wsize.x, wsize.y}, {0, wsize.y} });
		button->LeftPressed += [this](const MouseEvent& e) {
			if (e.just && _isButtonEnabled) {
				SceneManager.TransSceneTo(CommonData::CurrentBoard, 1.0, 0.3, true, SceneTransitionBehavior::Ignore);
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

		CreateScene<SoundPlayer>()
			->SetSound(LoadAsset<Sound>("sound/one_shot/clear_1.mp3"))
			->SetVolume(0.8f)
			->Play();

		CreateTweenGroup()->CreateTween(&background->alpha, 150, 0.5)->From(0);
		CreateTweenGroup()->CreateTween(&label->offset.x, 0, 1.0, Easing::EaseInCubic)->From(1500);

		CreateTimer(WAIT_TIME, [this]() {
			_isButtonEnabled = true;
			});
	}
}