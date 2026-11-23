#pragma once
#include <Scene.h>
#include <Scene/Text/TextLabel.h>

namespace Game {
	class DamagedEffect : public Scene {
	private:
		static constexpr double EFFECT_TIME = 2.0;
		int _damageAmount = 1;
		SharedPtr<TextLabel> _label;
	protected:
		void _Load() override;
	public:
		void SetDamageAmount(int amount);
	};
}