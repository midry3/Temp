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
		int _availableMP = 0;
		bool _isAlreadySelected = false;
		OutLineShaderBuffer _outlineBuffer;
		std::vector<SkillInfo> _skills;
		SharedPtr<GridContainer> _buttonGrid;
		SharedPtr<TextLabel> _skillNameLabel;
		SharedPtr<TextLabel> _primaryAttributeLabel;
		SharedPtr<TextLabel> _secondaryAttributeLabel;
		SharedPtr<TextLabel> _mpCostLabel;

		void RebuildButtons();
		void UpdateDescription(const SkillInfo& skill);
		void SetButtonAnimation(const SharedPtr<TextureButton>& button);
	protected:
		void _Load() override;
		void _Update(float dt) override;
	public:
		Signal<SkillInfo> SkillSelected;
		void SetSkills(const std::vector<SkillInfo>& skills);
		void SetAvailableMP(int mp) { _availableMP = mp; }
	};
}