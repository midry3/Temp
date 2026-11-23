#include "BattleUI.h"
#include <Scene/Container/Grid/GridContainer.h>
#include <Game/Data/SoundEffects.h>

namespace Game {
	void BattleUI::_Load() {
		AnimatedUI::_Load();
		SetInTime(0.5);
		SetOutTime(0.2);

		_outlineBuffer = OutLineShader::CreateConstantBuffer();

		const auto& commandGrid = CreateScene<GridContainer>();
		commandGrid->SetMatrixSize({ 1, 4 })
			->SetSize({ 200, 500 })
			->SetPosition({ 400, 100 });

		const auto& atkButton = commandGrid->CreateScene<TextureButton>();
		atkButton->SetTexture(LoadAsset<Texture>("ui/atk.png"))
			->SetScale(2)
			->SetPixelShader<OutLineShader>()
			->SetPixelShaderConstantBuffer(_outlineBuffer.handle);
		const auto& atkSE = CreateSelectSePlayer(atkButton.get());
		atkButton->LeftPressed += [this, atkSE](const MouseEvent& e) {
			if (e.just) {
				atkSE->Play();
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
			const auto& sprite = button->GetSprite();
			const auto& tg = button->CreateTweenGroup();
			tg->CreateTween(&sprite->scale, Vec2{1.2, 1.2}, 0.15);

			const auto& tg2 = button->CreateTweenGroup();
			tg2->CreateTween(&sprite->offset, Vec2{ -5, -5 }, 0.15);
			};
		button->Exited += [button](const MouseEvent& e) {
			button->KillAllTweenGroups();
			const auto& sprite = button->GetSprite();
			const auto& tg = button->CreateTweenGroup();
			tg->CreateTween(&sprite->scale, Vec2D_1, 0.15);

			const auto& tg2 = button->CreateTweenGroup();
			tg2->CreateTween(&sprite->offset, Vec2D_0, 0.15);
			};
	}
}