#include "BoardStatusPanel.h"

namespace Game {
	void BoardStatusPanel::_Load() {
		_panel = CreateScene<ColorRect>();
		_panel->SetSize({ PANEL_WIDTH, PANEL_HEIGHT })
			->SetColor({ 30, 50, 30 })
			->SetAlpha(200)
			->SetPosition({ 4, 4 });

		_icon = CreateScene<Sprite2D>();
		_icon->SetScale(1.5)
			->SetPosition({ 8, 8 });

		_nameLabel = CreateScene<TextLabel>();
		_nameLabel->BeginSetting()
			->SetFontSize(18)
			->EndSetting()
			->SetPosition({ 48, 10 });

		_hpLabel = CreateScene<TextLabel>();
		_hpLabel->BeginSetting()
			->SetFontSize(14)
			->EndSetting()
			->SetPosition({ 50, 36 });

		_hpBarBackground = CreateScene<Sprite2D>();
		_hpBarBackground->SetTexture(LoadAsset<Texture>("UI/hp_bar.png"))
			->SetModulate({ 255, 100, 100 })
			->SetPosition({ 50, 54 });
		_hpBar = CreateScene<ProgressSprite2D>();
		_hpBar->SetTexture(LoadAsset<Texture>("UI/hp_bar.png"))
			->SetModulate({ 100, 255, 100 })
			->SetPosition({ 50, 54 });
		_hpBar->SetProgress(1.0);

		_mpLabel = CreateScene<TextLabel>();
		_mpLabel->BeginSetting()
			->SetFontSize(14)
			->EndSetting()
			->SetPosition({ 50, 70 });

		_mpBarBackground = CreateScene<Sprite2D>();
		_mpBarBackground->SetTexture(LoadAsset<Texture>("UI/hp_bar.png"))
			->SetModulate({ 100, 100, 100 })
			->SetPosition({ 50, 88 });
		_mpBar = CreateScene<ProgressSprite2D>();
		_mpBar->SetTexture(LoadAsset<Texture>("UI/hp_bar.png"))
			->SetModulate({ 200, 100, 50 })
			->SetPosition({ 50, 88 });
		_mpBar->SetProgress(1.0);

		_otherLabel = CreateScene<TextLabel>();
		_otherLabel->BeginSetting()
			->SetFontSize(12)
			->EndSetting()
			->SetPosition({ 8, 106 });
	}

	void BoardStatusPanel::UpdateView() {
		if (not _chara) return;
		const auto& info = _chara->GetInfo();
		_icon->SetTexture(LoadAsset<Texture>(info.icon));
		_panel->SetColor(info.themeColor);
		_nameLabel->SetText(info.screenName);
		_hpLabel->SetText(std::format("HP: {} / {}", info.currentState.currentHP, info.stats.maxHP));
		_mpLabel->SetText(std::format("MP: {} / {}", info.currentState.currentMP, info.stats.maxMP));
		_otherLabel->SetText(std::format("ATK:{} DEF:{} MDEF:{} SPD:{}", info.currentState.atk, info.currentState.def, info.currentState.mdef, info.currentState.speed));
		_hpBar->SetProgress(static_cast<double>(info.currentState.currentHP) / info.stats.maxHP);
		_mpBar->SetProgress(static_cast<double>(info.currentState.currentMP) / info.stats.maxMP);
	}

	void BoardStatusPanel::SetCharacter(const SharedPtr<BattleCharacter>& chara) {
		_chara = chara;
		UpdateView();
	}
}
