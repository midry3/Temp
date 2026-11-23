#include "LoadingScene.h"
#include <Scene/Shape/ColorRect.h>
#include <chrono>
#include <format>

namespace Game {
	void LoadingScene::_Load() {
		Vector2D window = PropertyManager.GetSettingWindowSize();
		if (not (0.0 < window.x) || not (0.0 < window.y)) {
			window = PropertyManager.GetRealWindowSize();
		}

		const auto& background = CreateScene<ColorRect>();
		background->SetSize(window)
			->SetColor({ 16, 18, 24 })
			->SetAlpha(255);

		_progressLabel = CreateScene<TextLabel>();
		_progressLabel->BeginSetting()
			->SetFontSize(20)
			->SetForegroundColor({ 255, 255, 255 })
			->EndSetting()
			->SetText("Loading...")
			->SetPosition({ 24.0, window.y - 48.0 });
	}

	void LoadingScene::AddLoadTask(const std::function<void()>& task) {
		_loadTasks.push_back(task);
	}

	void LoadingScene::_Update(float dt) {
		if (_loadFinished) return;

		// 1フレームの予算内でタスクを処理する（最低でも1つは進める）
		const auto startTime = std::chrono::steady_clock::now();
		while (_nextTaskIndex < _loadTasks.size()) {
			if (_loadTasks[_nextTaskIndex]) _loadTasks[_nextTaskIndex]();
			++_nextTaskIndex;
			const auto elapsedMs = std::chrono::duration<double, std::milli>(
				std::chrono::steady_clock::now() - startTime).count();
			if (not (elapsedMs < FRAME_BUDGET_MS)) break;
		}

		if (_progressLabel) {
			_progressLabel->SetText(std::format("Loading... {} / {}",
				_nextTaskIndex, _loadTasks.size()));
		}

		if (not (_nextTaskIndex < _loadTasks.size())) {
			_loadFinished = true;
			OnLoadFinished();
		}
	}

	void LoadingScene::OnLoadFinished() {
	}
}
