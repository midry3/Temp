#include "UsingSkillPanel.h"

namespace Game {
	void UsingSkillPanel::_Load() {
		_panel = CreateScene<ColorRect>();
		_panel->SetSize({ 400, 200 })
			->SetColor({ 30, 30, 30 })
			->SetAlpha(100);
		_label = _panel->CreateScene<TextLabel>();
		_label->BeginSetting()
			->SetFontSize(50)
			->EndSetting()
			->SetPosition({ 20, 20 })
			->IgnoreParentAlpha();
	}

	void UsingSkillPanel::SetSkill(const SkillInfo& skill) {
		if (_label) {
			_label->SetText(skill.name);

			_panel->SetSize({ _label->GetSize().x + 100, 100});
		}
	}
}