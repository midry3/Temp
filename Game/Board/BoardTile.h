#pragma once
#include <Scene/Button/Button.h>
#include "BoardTypes.h"

class Sprite2D;
class Texture;
class ColorRect;

namespace Game {
	class BoardTile : public Button {
	private:
		// 点滅をオブジェクト等より上に描くためのオーバーレイの不透明度
		static constexpr int HINT_OVERLAY_ALPHA = 110;

		SharedPtr<Sprite2D> _sprite;
		SharedPtr<ColorRect> _hintOverlay;
		Vector2D _size;
		BoardTerrain _terrain = BoardTerrain::Normal;
		bool _selected = false;
		bool _moveHint = false;
		bool _enemyTargetHint = false;
		bool _occupiedHint = false;
		double _hintTime = 0.0;

		void UpdateColor();
		void UpdateHintOverlay(const Vector3D& color);
		void HideHintOverlay();

	protected:
		void _Load() override;
		void _Update(float dt) override;

	public:
		void SetTile(BoardTerrain terrain, const SharedPtr<Texture>& texture, const Vector2D& size);
		// 点滅オーバーレイの置き場（盤面の上位レイヤー）を設定する
		void SetHintLayer(const SharedPtr<Scene>& layer);
		[[nodiscard]] BoardTerrain GetTerrain() const { return _terrain; }
		[[nodiscard]] bool IsPassable() const { return _terrain != BoardTerrain::Disabled; }
		void SetSelected(bool selected);
		void SetMoveHint(bool moveHint);
		void SetEnemyTargetHint(bool enemyTargetHint);
		[[nodiscard]] bool HasEnemyTargetHint() const { return _enemyTargetHint; }
		void SetOccupiedHint(bool occupiedHint);
		[[nodiscard]] bool HasOccupiedHint() const { return _occupiedHint; }
	};
}
