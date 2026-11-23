#pragma once
#include <Scene.h>
#include <Scene/Text/TextLabel.h>
#include <functional>
#include <vector>

namespace Game {
	// 汎用ローディングシーン。登録された読み込みタスクをフレームに分散して処理し、
	// 全て終わったら OnLoadFinished を呼ぶ。使う側は継承して、タスクの登録と
	// 完了時の処理（シーン遷移など）を実装する。
	class LoadingScene : public Scene {
	private:
		// 1フレームで読み込みに使ってよい時間の目安（ミリ秒）
		static constexpr double FRAME_BUDGET_MS = 8.0;

		std::vector<std::function<void()>> _loadTasks;
		size_t _nextTaskIndex = 0;
		bool _loadFinished = false;
		SharedPtr<TextLabel> _progressLabel;
	protected:
		void _Load() override;
		void _Update(float dt) override;
		virtual void OnLoadFinished();
	public:
		void AddLoadTask(const std::function<void()>& task);
		[[nodiscard]] bool IsLoadFinished() const { return _loadFinished; }
	};
}
