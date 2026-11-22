#pragma once
#include <Scene/Button/Button.h>
#include <Scene/Text/TextLabel.h>
#include <vector>
#include "BoardTypes.h"

class Sprite2D;
class Texture;

namespace Game {
	class BoardUnit : public Button {
	private:
		BoardUnitData _data;
		SharedPtr<Sprite2D> _sprite;
		SharedPtr<TextLabel> _nameLabel;
		bool _intentHint = false;
		double _hintTime = 0.0;
		std::vector<BoardOffset> _relativeMoveOffsets;

	protected:
		void _Load() override;
		void _Update(float dt) override;

	public:
		void SetUnit(const BoardUnitData& data, const SharedPtr<Texture>& texture, const Vector2D& tileSize);
		void SetBoardPosition(int x, int y) { _data.x = x; _data.y = y; }
		[[nodiscard]] const BoardUnitData& GetData() const { return _data; }
		[[nodiscard]] bool IsAlly() const { return _data.team == BoardTeam::Ally; }
		void SetIntentHint(bool hint);
		[[nodiscard]] bool HasIntentHint() const { return _intentHint; }
		void SetRelativeMoveOffsets(const std::vector<BoardOffset>& offsets) { _relativeMoveOffsets = offsets; }
		[[nodiscard]] const std::vector<BoardOffset>& GetRelativeMoveOffsets() const { return _relativeMoveOffsets; }
	};
}
