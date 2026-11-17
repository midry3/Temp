#include "TestScene.h"
#include <Effekseer/EffekseerEmitter.h>

namespace Game {
	void TestScene::_Load() {
		const auto& emitter = CreateScene<EffekseerEmitter2D>();
		emitter->SetEffect("chara/plea/effect/fire.efk")
			->Play();
	}
}