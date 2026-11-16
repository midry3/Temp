#pragma once
#include <Scene/Container/Animation/AnimatedUI.h>
#include <Asset/Shader/OutLineShader.h>
#include <Scene/Button/TextureButton.h>
#include "SkillInfo.h"

namespace Game {
	class SkillSelect : public AnimatedUI {
	private:
		bool _isAlreadySelected = false;
		OutLineShaderBuffer _outlineBuffer;
		std::vector<SkillInfo> _skills;
		void SetButtonAnimation(const SharedPtr<TextureButton>& button);
	protected:
		void _Load() override;
		void _Update(float dt) override;
	public:
		Signal<SkillInfo> SkillSelected;
	};
}