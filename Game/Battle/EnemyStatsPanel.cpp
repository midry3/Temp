#include "EnemyStatsPanel.h"

namespace Game {
	void EnemyStatsPanel::_Load() {
		_panel = CreateScene<ColorRect>();
		_panel->SetSize({ 380, 90 })
			->SetColor({ 30, 50, 30 })
			->SetAlpha(200)
			->SetPosition({ 20, 20 });
		_nameLabel = CreateScene<TextLabel>();
		_nameLabel->BeginSetting()
			->SetFontSize(24)
			->EndSetting()
			->SetPosition({ 60, 40 });
		_hpTextLabel = CreateScene<TextLabel>();
		_hpTextLabel->BeginSetting()
			->SetFontSize(20)
			->EndSetting()
			->SetText("HP:")
			->SetPosition({ 70, 45 });
		_hpLabel = CreateScene<TextLabel>();
		_hpLabel->BeginSetting()
			->SetFontSize(20)
			->EndSetting()
			->SetPosition({ 170, 45 });

		_hpBarBackground = CreateScene<Sprite2D>();
		_hpBarBackground->SetTexture(LoadAsset<Texture>("UI/hp_bar.png"))
			->SetModulate({ 255, 100, 100 })
			->SetScale(1)
			->SetPosition({ 100, 45 });
		_hpBar = CreateScene<ProgressSprite2D>();
		_hpBar->SetTexture(LoadAsset<Texture>("UI/hp_bar.png"))
			->SetModulate({ 100, 255, 100 })
			->SetScale(1)
			->SetPosition({ 100, 45 });
		_hpBar->SetProgress(1.0);

		_otherLabel = CreateScene<TextLabel>();
		_otherLabel->BeginSetting()
			->SetFontSize(20)
			->EndSetting()
			->SetPosition({ 120, 65 });

		_icon = CreateScene<Sprite2D>();
		_icon->SetScale(2)
			->SetPosition({ 10, 10 });
	}

	void EnemyStatsPanel::UpdateView() {
		const auto& info = _chara->GetInfo();
		_icon->SetTexture(LoadAsset<Texture>(info.icon));
		_panel->SetColor(info.themeColor);
		_nameLabel->SetText(info.screenName);
		const auto off = Vec2{ _nameLabel->GetSize().x, 0 };
		_hpLabel->SetText(std::format("{} / {}", info.currentState.currentHP, info.stats.maxHP));
		_otherLabel->SetText(std::format("ATK: {}, SPD: {}\nDEF: {}, MDEF: {}", info.currentState.atk, info.currentState.speed, info.currentState.def, info.currentState.mdef));

		_hpTextLabel->SetOffset(off);
		_hpLabel->SetOffset(off);
		_hpBar->SetOffset(off);
		_hpBarBackground->SetOffset(off);

		_hpBar->SetProgress(static_cast<double>(_chara->GetState().currentHP) / _chara->GetInfo().stats.maxHP);
	}

	void EnemyStatsPanel::SetCharacter(const SharedPtr<BattleCharacter>& chara) {
		_chara = chara;
		UpdateView();
	}
}