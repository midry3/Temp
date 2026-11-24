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
		SharedPtr<Sprite2D> _vsSprite;
		bool _finished = false;

		void MoveToFormation();
		void MoveToCenter();
		void MoveBack();
		void Finish();
	protected:
		void _Load() override;
	public:
		void Setup(const std::vector<BattleStartIconData>& icons, const std::function<void()>& onFinished);
	};
}
