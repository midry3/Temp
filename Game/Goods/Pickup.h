#pragma once
#include <Scene.h>
#include <Scene/Texture/Sprite2D.h>

namespace Game {
	class Pickup : public Scene {
	private:
		SharedPtr<Sprite2D> _sprite;
	protected:
		void _Load() override;
	public:
		void SetPicture(const Path& path);
	};
}