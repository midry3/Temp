#pragma once
#include <Scene/Container/Animation/AnimatedUI.h>
#include <Asset/Shader/OutLineShader.h>
#include <Scene/Button/TextureButton.h>

namespace Game {
	class SkillSelect : public AnimatedUI {
	private:
		OutLineShaderBuffer _outlineBuffer;
		void SetButtonAnimation(const SharedPtr<TextureButton>& button);
	protected:
		void _Load() override;
		void _Update(float dt) override;
	};
}