#pragma once
#include <Scene.h>

namespace Game {
	// 敗北演出。WinScreen と同じ構成で、一定時間後にクリックで Board へ戻る
	class LoseScreen : public Scene {
	private:
		static constexpr double WAIT_TIME = 4.0;

		bool _isButtonEnabled = false;
	protected:
		void _Load() override;
	};
}
