#include "BattleUI.h"
#include <Scene/Container/Grid/GridContainer.h>

namespace Game {
	void BattleUI::_Load() {
		AnimatedUI::_Load();
		SetInTime(0.5);
		SetOutTime(0.2);

		_outlineBuffer = OutLineShader::CreateConstantBuffer();

		const auto& commandGrid = CreateScene<GridContainer>();
		commandGrid->SetMatrixSize({ 1, 4 })
			->SetSize({ 200, 500 })
			->SetPosition({ 100, 100 });

		const auto& atkButton = commandGrid->CreateScene<TextureButton>();
		atkButton->SetTexture(LoadAsset<Texture>("ui/atk.png"))
			->SetScale(2)
			->SetPixelShader<OutLineShader>()
			->SetPixelShaderConstantBuffer(_outlineBuffer.handle);
		atkButton->LeftPressed += [this](const MouseEvent& e) {
			if (e.just) {
				AtkPressed.Emit();
			}
			};
		SetButtonAnimation(atkButton);

		_outlineBuffer.param->glowColor[0] = 0.0f;
		_outlineBuffer.param->glowColor[1] = 0.6f;
		_outlineBuffer.param->glowColor[2] = 0.8f;
		_outlineBuffer.param->glowColor[3] = 1.0f;
		_outlineBuffer.param->outlineWidth = 2.0f;
		_outlineBuffer.param->textureSize[0] = atkButton->GetSprite()->GetRawSize().x;
		_outlineBuffer.param->textureSize[1] = atkButton->GetSprite()->GetRawSize().y;
		_outlineBuffer.param->time = 0.0f;
	}

	void BattleUI::_Update(float dt) {
		AnimatedUI::_Update(dt);
		_outlineBuffer.param->time += dt;
	}

	void BattleUI::SetButtonAnimation(const SharedPtr<TextureButton>& button) {
		button->Entered += [button](const MouseEvent& e) {
			button->KillAllTweenGroups();
			const auto& tg = button->CreateTweenGroup();
			const auto& sprite = button->GetSprite();
			tg->CreateTween(&sprite->rotation, PI / 12, 0.1)->From(0.0);
			tg->CreateTween(&sprite->rotation, -PI / 12, 0.1);
			tg->CreateTween(&sprite->rotation, 0, 0.05);
			};
	}
}