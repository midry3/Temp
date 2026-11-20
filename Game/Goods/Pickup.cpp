#include "Pickup.h"

namespace Game {
	void Pickup::_Load() {
		SetScale(3.0);

		const auto& pickup = CreateScene<Sprite2D>();
		pickup->SetTexture(LoadAsset<Texture>("goods/pickup.png"));
		_sprite = CreateScene<Sprite2D>();
		_sprite->SetPosition({ 15, 6 });
	}

	void Pickup::SetPicture(const Path& picture) {
		if (_sprite) {
			_sprite->SetTexture(LoadAsset<Texture>(picture));
		}
	}
}