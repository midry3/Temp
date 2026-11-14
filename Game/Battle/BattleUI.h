#pragma once
#include <Scene/Container/Animation/AnimatedUI.h>
#include <Scene/Button/TextureButton.h>
#include <Asset/Shader/OutLineShader.h>

namespace Game {
	class BattleUI : public AnimatedUI {
	private:
		OutLineShaderBuffer _outlineBuffer;

		void SetButtonAnimation(const SharedPtr<TextureButton>& button);
	protected:
		void _Load() override;
		void _Update(float dt) override;
	public:
		Signal<> AtkPressed;
	};
}