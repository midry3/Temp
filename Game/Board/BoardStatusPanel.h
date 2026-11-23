#pragma once
#include <Scene.h>
#include <Scene/Texture/Sprite2D.h>
#include <Scene/Shape/ColorRect.h>
#include <Scene/Texture/ProgressSprite2D.h>
#include <Scene/Text/TextLabel.h>
#include "../Battle/BattleCharacter/BattleCharacter.h"

namespace Game {
	// Board の情報パネル専用の味方ステータス表示。Battle の StatusPanel を参考に、
	// パネル幅に収まるサイズとフォントサイズを決め打ちで持つ（スケール調整はしない）。
	class BoardStatusPanel : public Scene {
	public:
		static constexpr int PANEL_WIDTH = 196;
		static constexpr int PANEL_HEIGHT = 124;
	private:
		SharedPtr<BattleCharacter> _chara;
		SharedPtr<ColorRect> _panel;
		SharedPtr<TextLabel> _nameLabel, _hpLabel, _mpLabel, _otherLabel;
		SharedPtr<Sprite2D> _icon, _hpBarBackground, _mpBarBackground;
		SharedPtr<ProgressSprite2D> _hpBar, _mpBar;
	protected:
		void _Load() override;
	public:
		void UpdateView();
		void SetCharacter(const SharedPtr<BattleCharacter>& chara);
	};
}
