#pragma once
#include "../LoadingScene.h"
#include "BattleBase.h"
#include <functional>
#include <vector>

namespace Game {
	// 戦闘用ローディングシーン。参加キャラ全員のアイコンと、全スキルの演出
	// タイムラインが参照するアセットを、戦闘シーン（BattleBase）の LoadAsset で
	// あらかじめ読み込む。読み込みが終わったら完了コールバックを呼ぶ。
	class BattleLoadingScene : public LoadingScene {
	private:
		std::function<void()> _onCompleted;
	protected:
		void OnLoadFinished() override;
	public:
		void Setup(const SharedPtr<BattleBase>& battleScene,
			const std::vector<SharedPtr<BattleCharacter>>& characters,
			const std::function<void()>& onCompleted);
	};
}
