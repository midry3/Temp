#pragma once
#include <Scene.h>

namespace Game {
	class BattleBackground : public Scene {
	private:
		double _time = 0.0;

	protected:
		void _Update(float dt) override;
		void _Draw() const override;
	};
}
