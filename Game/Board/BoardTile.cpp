#include "BoardTile.h"
#include <Scene/Texture/Sprite2D.h>
#include <Scene/Shape/ColorRect.h>
#include <Game/Data/SoundEffects.h>
#include <cmath>

namespace Game {
	void BoardTile::_Load() {
		Button::_Load();
		_sprite = CreateScene<Sprite2D>();
		const auto& clickSE = CreateSelectSePlayer(this);
		LeftPressed += [clickSE](const MouseEvent& event) {
			if (event.just) clickSE->Play();
			};
	}

	void BoardTile::SetTile(BoardTerrain terrain, const SharedPtr<Texture>& texture, const Vector2D& size) {
		_terrain = terrain;
		_size = size;
		SetArea({ Vector2D(0, 0), Vector2D(size.x, 0), size, Vector2D(0, size.y) });
		if (texture) {
			_sprite->SetTexture(texture);
			_sprite->Show();
			const auto textureSize = _sprite->GetRawSize();
			if (not (textureSize.x <= 0) && not (textureSize.y <= 0)) {
				_sprite->SetScale({ size.x / textureSize.x, size.y / textureSize.y });
			}
		}
		else {
			_sprite->Hide();
		}
		UpdateColor();
	}

	void BoardTile::SetHintLayer(const SharedPtr<Scene>& layer) {
		// 点滅はタイル自身の色替えだけだとオブジェクトに隠れるため、
		// 盤面の上位レイヤーに同色の半透明オーバーレイを重ねて表示する
		if (not layer || _hintOverlay) return;
		_hintOverlay = layer->CreateScene<ColorRect>();
		_hintOverlay->SetSize(_size);
		_hintOverlay->SetPosition(position);
		_hintOverlay->alpha = HINT_OVERLAY_ALPHA;
		_hintOverlay->Hide();
		UpdateColor();
	}

	void BoardTile::UpdateHintOverlay(const Vector3D& color) {
		if (not _hintOverlay) return;
		_hintOverlay->SetColor(color);
		_hintOverlay->Show();
	}

	void BoardTile::HideHintOverlay() {
		if (_hintOverlay) _hintOverlay->Hide();
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

	void BoardTile::SetEnemyTargetHint(bool enemyTargetHint) {
		_enemyTargetHint = enemyTargetHint;
		_hintTime = 0.0;
		UpdateColor();
	}

	void BoardTile::SetOccupiedHint(bool occupiedHint) {
		if (_occupiedHint == occupiedHint) return;
		_occupiedHint = occupiedHint;
		if (occupiedHint) _hintTime = 0.0;
		UpdateColor();
	}

	void BoardTile::_Update(float dt) {
		if (not _moveHint && not _enemyTargetHint && not _occupiedHint) return;
		_hintTime += dt;
		UpdateColor();
	}

	void BoardTile::UpdateColor() {
		if (not _sprite) return;
		if (_selected) {
			HideHintOverlay();
			_sprite->SetModulate({ 255, 230, 100 });
			return;
		}
		if (_moveHint && _enemyTargetHint) {
			const auto pulse = (std::sin(_hintTime * 6.0) + 1.0) / 2.0;
			const Vector3D color = Vector3D(255, 140, 140) * (1.0 - pulse) + Vector3D(255, 40, 40) * pulse;
			_sprite->SetModulate(color);
			UpdateHintOverlay(color);
			return;
		}
		if (_moveHint) {
			const auto pulse = (std::sin(_hintTime * 6.0) + 1.0) / 2.0;
			const Vector3D baseColor = _terrain == BoardTerrain::Other
				? Vector3D(180, 140, 80)
				: Vector3D(120, 160, 210);
			const Vector3D color = baseColor * (1.0 - pulse) + Vector3D(255, 255, 255) * pulse;
			_sprite->SetModulate(color);
			UpdateHintOverlay(color);
			return;
		}
		if (_enemyTargetHint) {
			const auto pulse = (std::sin(_hintTime * 6.0) + 1.0) / 2.0;
			const Vector3D baseColor = _terrain == BoardTerrain::Other
				? Vector3D(180, 140, 80)
				: Vector3D(120, 160, 210);
			const Vector3D color = baseColor * (1.0 - pulse) + Vector3D(255, 60, 60) * pulse;
			_sprite->SetModulate(color);
			UpdateHintOverlay(color);
			return;
		}
		if (_occupiedHint) {
			// 複数マス占有ユニットの占有マスは青系で点滅させる
			const auto pulse = (std::sin(_hintTime * 6.0) + 1.0) / 2.0;
			const Vector3D color = Vector3D(120, 160, 210) * (1.0 - pulse) + Vector3D(50, 110, 255) * pulse;
			_sprite->SetModulate(color);
			UpdateHintOverlay(color);
			return;
		}
		HideHintOverlay();
		// 通行不可マスも Board 上では通常マスと同じ色で表示する（黒くしない）
		if (_terrain == BoardTerrain::Other) {
			_sprite->SetModulate({ 180, 140, 80 });
		}
		else {
			_sprite->SetModulate({ 120, 160, 210 });
		}
	}
}
