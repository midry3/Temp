#include "SkillSelect.h"
#include <Scene/Container/Grid/GridContainer.h>
#include <Game/Data/SoundEffects.h>
#include <format>

namespace Game {
	namespace {
		std::string GetPrimaryAttributeDescription(const std::string& attribute) {
			if (attribute == "physical") return "物理: 武器や肉体による攻撃、DEFの影響を受ける";
			if (attribute == "special") return "特殊: 魔法や特殊能力による攻撃、MDEFの影響を受ける";
			return "第一属性: 不明";
		}

		std::string GetSecondaryAttributeDescription(const std::string& attribute) {
			if (attribute == "fire") return "火";
			if (attribute == "water") return "水";
			if (attribute == "nature") return "自然";
			return "無";
		}
	}

	void SkillSelect::_Load() {
		AnimatedUI::_Load();
		SetInTime(0.5);
		SetOutTime(0.5);
		_outlineBuffer = OutLineShader::CreateConstantBuffer();

		const auto& descriptionPanel = CreateScene<ColorRect>();
		descriptionPanel->SetSize({ 380, 500 })
			->SetColor({ 30, 30, 30 })
			->SetAlpha(230)
			->SetPosition({ 800, 100 });

		_skillNameLabel = CreateScene<TextLabel>();
		_skillNameLabel->BeginSetting()
			->SetFontSize(28)
			->SetWidth(360)
			->EndSetting()
			->SetPosition({ 825, 125 });

		_primaryAttributeLabel = CreateScene<TextLabel>();
		_primaryAttributeLabel->BeginSetting()
			->SetFontSize(20)
			->SetWidth(360)
			->EndSetting()
			->SetPosition({ 825, 200 });

		_secondaryAttributeLabel = CreateScene<TextLabel>();
		_secondaryAttributeLabel->BeginSetting()
			->SetFontSize(20)
			->SetWidth(360)
			->EndSetting()
			->SetPosition({ 825, 275 });

		_mpCostLabel = CreateScene<TextLabel>();
		_mpCostLabel->BeginSetting()
			->SetFontSize(20)
			->SetWidth(360)
			->EndSetting()
			->SetPosition({ 825, 350 });

		_buttonGrid = CreateScene<GridContainer>();
		_buttonGrid->SetMatrixSize({ 1, 1 })
			->SetSpace({ 0, 20 })
			->SetSize({ 200, 500 })
			->SetPosition({ 400, 100 });

		RebuildButtons();

		_outlineBuffer.param->glowColor[0] = 0.8f;
		_outlineBuffer.param->glowColor[1] = 0.6f;
		_outlineBuffer.param->glowColor[2] = 0.0f;
		_outlineBuffer.param->glowColor[3] = 1.0f;
		_outlineBuffer.param->outlineWidth = 2.0f;
		_outlineBuffer.param->textureSize[0] = 98.0f;
		_outlineBuffer.param->textureSize[1] = 48.0f;
		_outlineBuffer.param->time = 0.0f;
	}

	void SkillSelect::SetSkills(const std::vector<SkillInfo>& skills) {
		_skills = skills;
		if (_buttonGrid) RebuildButtons();
	}

	void SkillSelect::RebuildButtons() {
		if (not _buttonGrid) return;
		while (_buttonGrid->GetChildSceneCount() > 0) {
			_buttonGrid->DeleteScene(_buttonGrid->GetChildSceneCount() - 1);
		}
		_buttonGrid->SetMatrixSize({
			1, _skills.empty() ? 1.0 : static_cast<double>(_skills.size())
			});

		for (size_t i = 0; i < _skills.size(); i++) {
			const bool canUse = _skills[i].mpCost <= _availableMP;
			const auto& skillButton = _buttonGrid->CreateScene<TextureButton>();
			skillButton->SetTexture(LoadAsset<Texture>("ui/skill.png"))
				->SetScale(2)
				->SetPixelShader<OutLineShader>()
				->SetPixelShaderConstantBuffer(_outlineBuffer.handle);
			if (not canUse) {
				skillButton->SetAlpha(100)
					->SetModulate({100, 100, 100});
			}
			const auto& skillSE = CreateSelectSePlayer(skillButton.get());
			skillButton->LeftPressed += [this, skillButton, skillSE, i, canUse](const MouseEvent& e) {
				if (e.just && not _isAlreadySelected) {
					if (not canUse) return;
					skillSE->Play();
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
			skillButton->Entered += [this, i](const MouseEvent&) {
				UpdateDescription(_skills.at(i));
				};
			SetButtonAnimation(skillButton);
		}
	}

	void SkillSelect::UpdateDescription(const SkillInfo& skill) {
		_skillNameLabel->SetText(std::format("技名: {}", skill.name));
		_primaryAttributeLabel->SetText(GetPrimaryAttributeDescription(skill.primaryAttribute));
		_secondaryAttributeLabel->SetText(GetSecondaryAttributeDescription(skill.secondaryAttribute));
		_mpCostLabel->SetText(std::format("必要MP: {}", skill.mpCost));
	}

	void SkillSelect::_Update(float dt) {
		AnimatedUI::_Update(dt);
		_outlineBuffer.param->time += dt;
	}

	void SkillSelect::SetButtonAnimation(const SharedPtr<TextureButton>& button) {
		button->Entered += [button](const MouseEvent& e) {
			button->KillAllTweenGroups();
			const auto& sprite = button->GetSprite();

			const auto& tg = button->CreateTweenGroup();
			tg->CreateTween(&sprite->offset.y, -30.0, 0.1)->From(0.0);
			tg->CreateTween(&sprite->offset.y, 0.0, 0.1);
			};
	}
}