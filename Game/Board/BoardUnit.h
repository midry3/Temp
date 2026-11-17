#pragma once
#include <Scene/Button/Button.h>
#include "BoardTypes.h"

class Sprite2D;
class Texture;

namespace Game {
	class BoardUnit : public Button {
	private:
		BoardUnitData _data;
		SharedPtr<Sprite2D> _sprite;

	protected:
		void _Load() override;

	public:
		void SetUnit(const BoardUnitData& data, const SharedPtr<Texture>& texture, const Vector2D& tileSize);
		void SetBoardPosition(int x, int y) { _data.x = x; _data.y = y; }
		[[nodiscard]] const BoardUnitData& GetData() const { return _data; }
		[[nodiscard]] bool IsAlly() const { return _data.team == BoardTeam::Ally; }
	};
}
