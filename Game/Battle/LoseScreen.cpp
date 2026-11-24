#include "LoseScreen.h"
#include <Scene/Shape/ColorRect.h>
#include <Scene/Text/TextLabel.h>
#include <Scene/Button/Button.h>
#include <Game/StageSelect.h>
#include <Game/Data/CommonData.h>

namespace Game {
	void LoseScreen::_Load() {
		const auto wsize = PropertyManager.GetSettingWindowSize();
		const auto& button = CreateScene<Button>();
		button->SetArea({ {0, 0}, {wsize.x, 0}, {wsize.x, wsize.y}, {0, wsize.y} });
		button->LeftPressed += [this](const MouseEvent& e) {
			if (e.just && _isButtonEnabled) {
				if (CommonData::LastBattleCanContinue && CommonData::CurrentBoard) {
					// 戦闘に参加していない味方が残っていれば Board へ戻ってステージを継続する
					SceneManager.TransSceneTo(CommonData::CurrentBoard, 1.0, 0.3, true, SceneTransitionBehavior::Ignore);
				}
				else {
					// 味方が残っていなければステージ選択へ移行する
					SceneManager.TransSceneTo(MakeShared<StageSelect>(), 1.0, 0.3, true, SceneTransitionBehavior::Ignore);
				}
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
			->SetText("You Lose...")
			->SetPosition({ 300, 300 });

		CreateTweenGroup()->CreateTween(&background->alpha, 150, 0.5)->From(0);
		CreateTweenGroup()->CreateTween(&label->offset.x, 0, 1.0, Easing::EaseInCubic)->From(1500);

		CreateTimer(WAIT_TIME, [this]() {
			_isButtonEnabled = true;
			});
	}
}
