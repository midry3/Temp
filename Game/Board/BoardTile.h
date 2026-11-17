#pragma once
#include <Scene/Button/Button.h>
#include "BoardTypes.h"

class Sprite2D;
class Texture;

namespace Game {
	class BoardTile : public Button {
	private:
		SharedPtr<Sprite2D> _sprite;
		BoardTerrain _terrain = BoardTerrain::Normal;
		bool _selected = false;
		bool _moveHint = false;
		double _hintTime = 0.0;

		void UpdateColor();

	protected:
		void _Load() override;
		void _Update(float dt) override;

	public:
		void SetTile(BoardTerrain terrain, const SharedPtr<Texture>& texture, const Vector2D& size);
		[[nodiscard]] BoardTerrain GetTerrain() const { return _terrain; }
		[[nodiscard]] bool IsPassable() const { return _terrain != BoardTerrain::Disabled; }
		void SetSelected(bool selected);
		void SetMoveHint(bool moveHint);
	};
}
