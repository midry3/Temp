#pragma once
#include <Scene/Container/Animation/AnimatedUI.h>
#include <Asset/Shader/OutLineShader.h>
#include <Scene/Shape/ColorRect.h>
#include <Scene/Text/TextLabel.h>
#include <Scene/Button/TextureButton.h>
#include "SkillInfo.h"

class GridContainer;

namespace Game {
	class SkillSelect : public AnimatedUI {
	private:
		// 「戻る」ボタンの配置（スキル一覧の下）
		static constexpr double BACK_BUTTON_X = 400.0;
		static constexpr double BACK_BUTTON_Y = 612.0;
		static constexpr double BACK_BUTTON_SCALE = 0.7;

		// スキル名ラベル（各スキルボタンの中央）のデザイン定数
		static constexpr int SKILL_NAME_FONT_SIZE = 18;
		static constexpr int SKILL_NAME_COLOR_R = 255;
		static constexpr int SKILL_NAME_COLOR_G = 255;
		static constexpr int SKILL_NAME_COLOR_B = 255;

		int _availableMP = 0;
		bool _isAlreadySelected = false;
		OutLineShaderBuffer _outlineBuffer;
		std::vector<SkillInfo> _skills;
		SharedPtr<GridContainer> _buttonGrid;
		SharedPtr<TextLabel> _skillNameLabel;
		SharedPtr<TextLabel> _primaryAttributeLabel;
		SharedPtr<TextLabel> _secondaryAttributeLabel;
		SharedPtr<TextLabel> _mpCostLabel;
		SharedPtr<TextLabel> _descriptionLabel;

		void RebuildButtons();
		void UpdateDescription(const SkillInfo& skill);
		void SetButtonAnimation(const SharedPtr<TextureButton>& button);
	protected:
		void _Load() override;
		void _Update(float dt) override;
	public:
		Signal<SkillInfo> SkillSelected;
		Signal<> BackPressed;

		// BattleUI と同様、切り替え前に子シーンのトゥイーンを止めて残り表示を防ぐ
		void ShowUI();
		void HideUI();

		void SetSkills(const std::vector<SkillInfo>& skills);
		void SetAvailableMP(int mp) { _availableMP = mp; }
	};
}