#pragma once
#include <Scene/Sound/SoundPlayer.h>

namespace Game {
	// 選択できるボタン用のSE（仮）。差し替える場合はこの定数を変更する。
	inline constexpr const char* SELECT_SE = "sound/one_shot/select_2.oga";

	// 共闘ラインが新しく結ばれたときのSE（仮）。差し替える場合はこの定数を変更する。
	inline constexpr const char* COOP_LINE_SE = "sound/one_shot/select_2.oga";

	// 選択SE再生用の SoundPlayer を子シーンとして作成する
	inline SharedPtr<SoundPlayer> CreateSelectSePlayer(Scene* scene) {
		const auto& player = scene->CreateScene<SoundPlayer>();
		player->SetSound(scene->LoadAsset<Sound>(SELECT_SE));
		return player;
	}
}
