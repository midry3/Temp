#include "BoardUnit.h"
#include <Scene/Texture/Sprite2D.h>

namespace Game {
	void BoardUnit::_Load() {
		Button::_Load();
		_sprite = CreateScene<Sprite2D>();
	}

	void BoardUnit::SetUnit(const BoardUnitData& data, const SharedPtr<Texture>& texture, const Vector2D& tileSize) {
		_data = data;
		SetArea({
			Vector2D(0, 0),
			Vector2D(tileSize.x, 0),
			tileSize,
			Vector2D(0, tileSize.y)
			});
		_sprite->SetTexture(texture);
		const auto textureSize = _sprite->GetRawSize();
		if (not (textureSize.x <= 0) && not (textureSize.y <= 0)) {
			const auto scale = Min(tileSize.x / textureSize.x, tileSize.y / textureSize.y) * 0.8;
			_sprite->SetScale(scale);
			_sprite->SetPosition(tileSize * 0.1);
		}
	}
}
