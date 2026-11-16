#include "SkillSelect.h"
#include <Scene/Container/Grid/GridContainer.h>

namespace Game {
	void SkillSelect::_Load() {
		AnimatedUI::_Load();
		SetInTime(0.5);
		SetOutTime(0.5);
		_outlineBuffer = OutLineShader::CreateConstantBuffer();
		_skills = {
			{ "Attack", "physical", "battle/attack.lua" },
			{ "Fire", "fire", "battle/fire.lua" },
			{ "Ice", "ice", "battle/ice.lua" },
			{ "Heal", "holy", "battle/heal.lua" }
		};

		const auto& buttonGrid = CreateScene<GridContainer>();
		buttonGrid->SetMatrixSize({ 1, 4 })
			->SetSpace({ 0, 20 })
			->SetSize({ 200, 500 })
			->SetPosition({ 100, 100 });

		for (int i = 0; i < 4; i++) {
			const auto& skillButton = buttonGrid->CreateScene<TextureButton>();
			skillButton->SetTexture(LoadAsset<Texture>("ui/skill.png"))
				->SetScale(2)
				->SetPixelShader<OutLineShader>()
				->SetPixelShaderConstantBuffer(_outlineBuffer.handle);
			skillButton->LeftPressed += [this, skillButton, i](const MouseEvent& e) {
				if (e.just && not _isAlreadySelected) {
					_isAlreadySelected = true;
					const auto& tg = CreateTweenGroup();
					tg->CreateTween(&skillButton->alpha, 50, 0.1);
					tg->CreateTween(&skillButton->alpha, 200, 0.1);
					tg->SetLoop(5);
					tg->Completed += [this, i]() {
						SkillSelected.Emit(_skills.at(i));
						Out();
						_isAlreadySelected = false;
						};
				}
				};
			SetButtonAnimation(skillButton);
		}
		_outlineBuffer.param->glowColor[0] = 0.8f;
		_outlineBuffer.param->glowColor[1] = 0.6f;
		_outlineBuffer.param->glowColor[2] = 0.0f;
		_outlineBuffer.param->glowColor[3] = 1.0f;
		_outlineBuffer.param->outlineWidth = 2.0f;
		_outlineBuffer.param->textureSize[0] = 98.0f;
		_outlineBuffer.param->textureSize[1] = 48.0f;
		_outlineBuffer.param->time = 0.0f;
	}

	void SkillSelect::_Update(float dt) {
		AnimatedUI::_Update(dt);
		_outlineBuffer.param->time += dt;
	}

	void SkillSelect::SetButtonAnimation(const SharedPtr<TextureButton>& button) {
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