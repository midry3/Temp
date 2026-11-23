#pragma once
#include <Scene.h>
#include <Scene/Texture/Sprite2D.h>
#include <Scene/Shape/ColorRect.h>
#include <Scene/Texture/ProgressSprite2D.h>
#include <Scene/Text/TextLabel.h>
#include "BattleCharacter/BattleCharacter.h"

namespace Game {
	class StatusPanel : public Scene {
	private:
		SharedPtr<BattleCharacter> _chara;
		SharedPtr<ColorRect> _panel;
		SharedPtr<TextLabel> _nameLabel, _hpLabel, _hpTextLabel, _mpLabel, _mpTextLabel, _otherLabel;
		SharedPtr<Sprite2D> _icon, _hpBarBackground, _mpBarBackground;
		SharedPtr<ProgressSprite2D> _hpBar, _mpBar;
	protected:
		void  _Load() override;
	public:
		void UpdateView();
		void SetCharacter(const SharedPtr<BattleCharacter>& chara);
		void UseMP(int mp);
	};
}