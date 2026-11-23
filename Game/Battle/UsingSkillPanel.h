#pragma once
#include <Scene.h>
#include <Scene/Shape/ColorRect.h>
#include <Scene/Text/TextLabel.h>
#include "SkillInfo.h"

namespace Game {
	class UsingSkillPanel : public Scene {
	private:
		SharedPtr<TextLabel> _label;
		SharedPtr<ColorRect> _panel;
	protected:
		void _Load() override;
	public:
		void SetSkill(const SkillInfo& skill);
	};
}