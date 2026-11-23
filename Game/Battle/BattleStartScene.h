#pragma once
#include <Scene.h>
#include <Scene/Texture/Sprite2D.h>
#include <functional>
#include <string>
#include <vector>

namespace Game {
	struct BattleStartIconData {
		std::string texture;
		Vector2D boardPosition;
		double tileSize = 0.0;
		bool isEnemy = false;
	};

	class BattleStartScene : public Scene {
	private:
		struct Icon {
			SharedPtr<Sprite2D> sprite;
			Vector2D formationPosition;
			Vector2D centerPosition;
			bool isEnemy = false;
		};

		std::vector<BattleStartIconData> _iconData;
		std::vector<Icon> _icons;
		std::function<void()> _onFinished;
		std::function<bool()> _preloadStep;
		bool _preloadDone = true;
		bool _waitingPreload = false;
		bool _finished = false;

		void MoveToFormation();
		void MoveToCenter();
		void MoveBack();
		void Finish();
	protected:
		void _Load() override;
		void _Update(float dt) override;
	public:
		// preloadStep は1回で1単位の事前ロードを進め、完了したら true を返す。
		// 演出が終わってもロードが完了するまでは戦闘へ遷移しない。
		void Setup(const std::vector<BattleStartIconData>& icons, const std::function<void()>& onFinished,
			const std::function<bool()>& preloadStep = nullptr);
	};
}
