#include "DamagedEffect.h"

namespace Game {
	void DamagedEffect::_Load() {
		const double x = GetRandomInt(-200, 200);
		_label = CreateScene<TextLabel>();
		_label->BeginSetting()
			->SetFontSize(40)
			->SetForegroundColor({ 255, 50, 50 })
			->EndSetting()
			->SetPosition({ x, -50 });

		CreateTweenGroup()->CreateTween(&_label->offset, GetRandomIntVector2D({ -100, -200 }, {100, -20}), EFFECT_TIME, Easing::EaseOutCubic);
		CreateTweenGroup()->CreateTween(&_label->alpha, 0, EFFECT_TIME, Easing::EaseInCubic);

		CreateTimer(EFFECT_TIME, [this]() {
			Free();
			});
	}

	void DamagedEffect::SetDamageAmount(int amount) {
		_damageAmount = amount;
		if (_label) {
			_label->SetText(std::format("-{}", _damageAmount));
		}
	}
}