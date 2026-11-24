#include "BoardUnit.h"
#include <Scene/Texture/Sprite2D.h>
#include <Scene/Sound/SoundPlayer.h>
#include <Game/Data/SoundEffects.h>
#include <cmath>

namespace Game {
	void BoardUnit::_Load() {
		Button::_Load();
		_sprite = CreateScene<Sprite2D>();
		_nameLabel = CreateScene<TextLabel>();
		_nameLabel->SetFontSize(14)->SetForegroundColor({ 20, 20, 20 });

		Entered += [this](const MouseEvent& event) {
			_isMouseEntered = true;
		};
		Exited += [this](const MouseEvent& event) {
			_isMouseEntered = false;
			if (not _intentHint) {
				_sprite->SetModulate({ 255, 255, 255 });
			}
			};

		const auto& clickSE = CreateSelectSePlayer(this);
		LeftPressed += [clickSE](const MouseEvent& event) {
			if (event.just) clickSE->Play();
			};
	}

	void BoardUnit::_Update(float dt) {
		Button::_Update(dt);
		if (not _intentHint && not _isMouseEntered) return;
		_hintTime += dt;
		if (_sprite) {
			if (_intentHint) {
				const auto pulse = (std::sin(_hintTime * 6.0) + 1.0) / 2.0;
				_sprite->SetModulate(Vector3D(255, 60, 60) * pulse + Vector3D(255, 255, 255) * (1.0 - pulse));
			}
			else if (_isMouseEntered) {
				const auto pulse = (std::sin(_hintTime * 6.0) + 1.0) / 2.0;
				_sprite->SetModulate(Vector3D(60, 60, 60) * pulse + Vector3D(255, 255, 255) * (1.0 - pulse));
			}
		}
	}

	void BoardUnit::SetUnit(const BoardUnitData& data, const SharedPtr<Texture>& texture, const Vector2D& tileSize) {
		_data = data;
		SetArea({
			Vector2D(0, 0),
			Vector2D(tileSize.x, 0),
			tileSize,
			Vector2D(0, tileSize.y)
			});
		if (texture) _sprite->SetTexture(texture);
		else _sprite->Hide();
		if (not data.displayName.empty()) {
			_nameLabel->SetText(data.displayName)->SetWidth(static_cast<int>(tileSize.x));
			_nameLabel->SetPosition({ 2.0, tileSize.y * 0.76 });
		}
		else _nameLabel->Hide();
		const auto textureSize = _sprite->GetRawSize();
		if (not (textureSize.x <= 0) && not (textureSize.y <= 0)) {
			const auto scale = Min(tileSize.x / textureSize.x, tileSize.y / textureSize.y) * 0.8;
			_sprite->SetScale(scale);
			_sprite->SetPosition(tileSize * 0.1 + Vector2D(data.iconOffsetX, data.iconOffsetY));
		}
	}

	void BoardUnit::SetIntentHint(bool hint) {
		_intentHint = hint;
		_hintTime = 0.0;
		if (not _intentHint && _sprite) {
			_sprite->SetModulate({ 255, 255, 255 });
		}
	}
}
