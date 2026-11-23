#pragma once
#include <Scene.h>
#include <Scene/Texture/Sprite2D.h>
#include <Scene/Shape/ColorRect.h>
#include <Scene/Texture/ProgressSprite2D.h>
#include <Scene/Text/TextLabel.h>
#include "../Battle/BattleCharacter/BattleCharacter.h"

namespace Game {
	// Board の情報パネル専用の敵ステータス表示。Battle の EnemyStatsPanel を参考に、
	// フォントサイズを決め打ちで持つ（スケール調整はしない）。
	class BoardEnemyStatusPanel : public Scene {
	public:
		static constexpr int PANEL_WIDTH = 380;
		static constexpr int PANEL_HEIGHT = 100;
	private:
		SharedPtr<BattleCharacter> _chara;
		SharedPtr<ColorRect> _panel;
		SharedPtr<TextLabel> _nameLabel, _hpLabel, _otherLabel;
		SharedPtr<Sprite2D> _icon, _hpBarBackground;
		SharedPtr<ProgressSprite2D> _hpBar;
	protected:
		void _Load() override;
	public:
		void UpdateView();
		void SetCharacter(const SharedPtr<BattleCharacter>& chara);
	};
}
