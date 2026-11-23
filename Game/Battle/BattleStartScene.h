#pragma once
#include <Scene.h>
#include <Scene/Texture/Sprite2D.h>
#include <functional>
#include <string>
#include <vector>

namespace Game {
	// 戦闘開始演出用のアイコン情報。位置と大きさは Board 上の表示（ウィンドウ座標）に合わせる。
	struct BattleStartIconData {
		std::string texture;
		Vector2D boardPosition;
		double tileSize = 0.0;
		bool isEnemy = false;
	};

	// 戦闘開始演出シーン（暫定）。白背景に敵味方のアイコンを Board と同じ位置・大きさで表示し、
	// 味方は左・敵は右へ移動 → 中央で一度ぶつかり → 弾かれて左右に戻り、戦闘シーンへ移行する。
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
