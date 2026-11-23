#include "BattleLoadingScene.h"
#include "BattleAttack.h"
#include <Asset/Sound/Sound.h>
#include <Effekseer/EffekseerAsset.h>

namespace Game {
	void BattleLoadingScene::Setup(const SharedPtr<BattleBase>& battleScene,
		const std::vector<SharedPtr<BattleCharacter>>& characters,
		const std::function<void()>& onCompleted) {
		_onCompleted = onCompleted;
		if (not battleScene) return;

		for (const auto& character : characters) {
			if (not character) continue;

			const auto icon = character->GetInfo().icon.string();
			if (not icon.empty()) {
				AddLoadTask([battleScene, icon]() {
					const auto& asset = battleScene->LoadAsset<Texture>(icon);
					if (asset) asset->Load();
					});
			}

			for (const auto& skill : character->GetSkills()) {
				if (skill.directionFile.empty()) continue;
				AddLoadTask([battleScene, character, skill]() {
					const auto assets = BattleAttack::CollectSkillAssets(character, skill);
					// LoadAsset の登録だけでは実ロードが走らないアセット
					// （Effekseer 等）があるため、Load() を明示的に呼ぶ
					for (const auto& effect : assets.effects) {
						const auto& asset = battleScene->LoadAsset<EffekseerAsset>(effect);
						if (asset) asset->Load();
					}
					for (const auto& texture : assets.textures) {
						const auto& asset = battleScene->LoadAsset<Texture>(texture);
						if (asset) asset->Load();
					}
					for (const auto& sound : assets.sounds) {
						const auto& asset = battleScene->LoadAsset<Sound>(sound);
						if (asset) asset->Load();
					}
					});
			}
		}
	}

	void BattleLoadingScene::OnLoadFinished() {
		if (_onCompleted) _onCompleted();
	}
}
