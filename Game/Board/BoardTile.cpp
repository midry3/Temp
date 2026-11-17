#include "BoardTile.h"
#include <Scene/Texture/Sprite2D.h>
#include <cmath>

namespace Game {
	void BoardTile::_Load() {
		Button::_Load();
		_sprite = CreateScene<Sprite2D>();
	}

	void BoardTile::SetTile(BoardTerrain terrain, const SharedPtr<Texture>& texture, const Vector2D& size) {
		_terrain = terrain;
		SetArea({ Vector2D(0, 0), Vector2D(size.x, 0), size, Vector2D(0, size.y) });
		_sprite->SetTexture(texture);
		const auto textureSize = _sprite->GetRawSize();
		if (not (textureSize.x <= 0) && not (textureSize.y <= 0)) {
			_sprite->SetScale({ size.x / textureSize.x, size.y / textureSize.y });
		}
		UpdateColor();
	}

	void BoardTile::SetSelected(bool selected) {
		_selected = selected;
		UpdateColor();
	}

	void BoardTile::SetMoveHint(bool moveHint) {
		_moveHint = moveHint;
		_hintTime = 0.0;
		UpdateColor();
	}

	void BoardTile::_Update(float dt) {
		if (not _moveHint) return;
		_hintTime += dt;
		UpdateColor();
	}

	void BoardTile::UpdateColor() {
		if (not _sprite) return;
		if (_selected) {
			_sprite->SetModulate({ 255, 230, 100 });
			return;
		}
		if (_moveHint) {
			const auto pulse = (std::sin(_hintTime * 6.0) + 1.0) / 2.0;
			const Vector3D baseColor = _terrain == BoardTerrain::Other
				? Vector3D(180, 140, 80)
				: Vector3D(120, 160, 210);
			_sprite->SetModulate(baseColor * (1.0 - pulse) + Vector3D(255, 255, 255) * pulse);
			return;
		}
		if (_terrain == BoardTerrain::Disabled) {
			_sprite->SetModulate({ 80, 80, 80 });
		}
		else if (_terrain == BoardTerrain::Other) {
			_sprite->SetModulate({ 180, 140, 80 });
		}
		else {
			_sprite->SetModulate({ 120, 160, 210 });
		}
	}
}
