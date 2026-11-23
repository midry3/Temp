#include "BoardEnemyStatusPanel.h"

namespace Game {
	void BoardEnemyStatusPanel::_Load() {
		_panel = CreateScene<ColorRect>();
		_panel->SetSize({ PANEL_WIDTH, PANEL_HEIGHT })
			->SetColor({ 30, 50, 30 })
			->SetAlpha(200)
			->SetPosition({ 4, 4 });

		_icon = CreateScene<Sprite2D>();
		_icon->SetScale(2)
			->SetPosition({ 8, 8 });

		_nameLabel = CreateScene<TextLabel>();
		_nameLabel->BeginSetting()
			->SetFontSize(20)
			->EndSetting()
			->SetPosition({ 56, 12 });

		_hpLabel = CreateScene<TextLabel>();
		_hpLabel->BeginSetting()
			->SetFontSize(16)
			->EndSetting()
			->SetPosition({ 56, 40 });

		_hpBarBackground = CreateScene<Sprite2D>();
		_hpBarBackground->SetTexture(LoadAsset<Texture>("UI/hp_bar.png"))
			->SetModulate({ 255, 100, 100 })
			->SetPosition({ 56, 62 });
		_hpBar = CreateScene<ProgressSprite2D>();
		_hpBar->SetTexture(LoadAsset<Texture>("UI/hp_bar.png"))
			->SetModulate({ 100, 255, 100 })
			->SetPosition({ 56, 62 });
		_hpBar->SetProgress(1.0);

		_otherLabel = CreateScene<TextLabel>();
		_otherLabel->BeginSetting()
			->SetFontSize(14)
			->EndSetting()
			->SetPosition({ 8, 80 });
	}

	void BoardEnemyStatusPanel::UpdateView() {
		if (not _chara) return;
		const auto& info = _chara->GetInfo();
		_icon->SetTexture(LoadAsset<Texture>(info.icon));
		_panel->SetColor(info.themeColor);
		_nameLabel->SetText(info.screenName);
		_hpLabel->SetText(std::format("HP: {} / {}", info.currentState.currentHP, info.stats.maxHP));
		_otherLabel->SetText(std::format("ATK:{} SPD:{}  DEF:{} MDEF:{}", info.currentState.atk, info.currentState.speed, info.currentState.def, info.currentState.mdef));
		_hpBar->SetProgress(static_cast<double>(info.currentState.currentHP) / info.stats.maxHP);
	}

	void BoardEnemyStatusPanel::SetCharacter(const SharedPtr<BattleCharacter>& chara) {
		_chara = chara;
		UpdateView();
	}
}
