#pragma once
#include <Scene.h>

namespace Game {
	class WinScreen : public Scene {
	private:
		static constexpr double WAIT_TIME = 4.0;

		bool _isButtonEnabled = false;
	protected:
		void _Load() override;
	};
}