#pragma once
#include <Scene/Container/Animation/AnimatedUI.h>
#include <Scene/Button/TextureButton.h>
#include <Asset/Shader/OutLineShader.h>

namespace Game {
	class BattleUI : public AnimatedUI {
	private:
		// 「戻る」ボタンの配置（降参ボタンの右下あたりに小さめに置く）
		static constexpr double BACK_BUTTON_X = 608.0;
		static constexpr double BACK_BUTTON_Y = 420.0;
		static constexpr double BACK_BUTTON_SCALE = 0.7;

		OutLineShaderBuffer _outlineBuffer;
		SharedPtr<TextureButton> _backButton;
		bool _backEnabled = false;

		void SetButtonAnimation(const SharedPtr<TextureButton>& button);
	protected:
		void _Load() override;
		void _Update(float dt) override;
	public:
		Signal<> AtkPressed;
		Signal<> PassPressed;
		Signal<> SurrenderPressed;
		Signal<> BackPressed;

		// 「戻る」の有効・無効を切り替える（無効時は暗く表示し、押せない）
		void SetBackEnabled(bool enabled);
	};
}