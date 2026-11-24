#include "BattleStartScene.h"
#include <Scene/Shape/ColorRect.h>
#include <Scene/Sound/SoundPlayer.h>
#include <Asset/AssetManager.h>

namespace Game {
	void BattleStartScene::Setup(const std::vector<BattleStartIconData>& icons, const std::function<void()>& onFinished) {
		_iconData = icons;
		_onFinished = onFinished;
	}

	void BattleStartScene::_Load() {
		Vector2D window = PropertyManager.GetSettingWindowSize();
		if (not (0.0 < window.x) || not (0.0 < window.y)) {
			window = PropertyManager.GetRealWindowSize();
		}

		const auto& background = CreateScene<ColorRect>();
		background->SetSize(window)
			->SetColor({ 255, 255, 255 })
			->SetAlpha(255);

		int allyCount = 0;
		int enemyCount = 0;
		for (const auto& data : _iconData) {
			if (data.isEnemy) enemyCount++;
			else allyCount++;
		}

		int allyIndex = 0;
		int enemyIndex = 0;
		for (const auto& data : _iconData) {
			const auto& sprite = CreateScene<Sprite2D>();
			sprite->SetTexture(LoadAsset<Texture>(data.texture));

			const auto textureSize = sprite->GetRawSize();
			if (not (textureSize.x <= 0) && not (textureSize.y <= 0)) {
				const auto scale = Min(data.tileSize / textureSize.x, data.tileSize / textureSize.y) * 0.8;
				sprite->SetScale(scale);
			}
			sprite->SetPosition(data.boardPosition + Vec2D_1 * data.tileSize * 0.1 );

			const int count = data.isEnemy ? enemyCount : allyCount;
			const int index = data.isEnemy ? enemyIndex++ : allyIndex++;
			const double slot = data.tileSize;
			const double totalHeight = count * slot + (count - 1) * 12.0;
			const double slotY = window.y * 0.5 - totalHeight * 0.5 + index * (slot + 5.0);
			const double offsetInTile = slot * 0.05;

			Icon icon;
			icon.sprite = sprite;
			icon.isEnemy = data.isEnemy;
			icon.formationPosition = data.isEnemy
				? Vector2D(window.x - 120.0 - slot + offsetInTile, slotY + offsetInTile)
				: Vector2D(120.0 + offsetInTile, slotY + offsetInTile);
			icon.centerPosition = data.isEnemy
				? Vector2D(window.x * 0.5 + 6.0 + offsetInTile, slotY + offsetInTile)
				: Vector2D(window.x * 0.5 - slot - 6.0 + offsetInTile, slotY + offsetInTile);
			_icons.push_back(icon);
		}
		/*
		CreateScene<SoundPlayer>()->SetSound(LoadAsset<Sound>("sound/one_shot/start_battle_1.oga"))
			->SetVolume(0.8f)
			->Play();*/
		_vsSprite = CreateScene<Sprite2D>();
		_vsSprite->SetTexture(LoadAsset<Texture>("UI/vs.png"))
			->SetPosition({window.x * 0.47, window.y * 0.45})
			->Disable();

		CreateTimer(0.25, [this]() { MoveToFormation(); });
	}

	void BattleStartScene::MoveToFormation() {
		for (const auto& icon : _icons) {
			const auto& tg = icon.sprite->CreateTweenGroup();
			tg->CreateTween(&icon.sprite->position, icon.formationPosition, 0.6, Easing::EaseInOutCubic);
		}
		CreateTimer(0.7, [this]() { MoveToCenter(); });
	}

	void BattleStartScene::MoveToCenter() {
		for (const auto& icon : _icons) {
			const auto& tg = icon.sprite->CreateTweenGroup();
			tg->CreateTween(&icon.sprite->position, icon.centerPosition, 0.4, Easing::EaseInCubic);
		}
		CreateTweenGroup()->CreateTween(&Camera.zoom, Vec2{ 3.5, 3.5 }, 0.7);
		CreateTimer(0.2, [this]() {
			CreateScene<SoundPlayer>()->SetSound(LoadAsset<Sound>("sound/one_shot/start_battle_2.mp3"))
				->SetVolume(0.8f)
				->Play();
			_vsSprite->Enable();
			const auto& tg = CreateTweenGroup();
			tg->CreateTween(&_vsSprite->scale, Vec2{2.0, 2.0}, 0.2)->From(Vec2D_1);

			const auto& tg2 = CreateTweenGroup();
			tg2->CreateTween(&_vsSprite->offset, Vec2{ -20, -20 }, 0.2)->From(Vec2D_1);
			});
		CreateTimer(2.0, [this]() { Finish(); });
	}

	void BattleStartScene::MoveBack() {
		for (const auto& icon : _icons) {
			const auto& tg = icon.sprite->CreateTweenGroup();
			tg->CreateTween(&icon.sprite->position, icon.formationPosition, 0.35, Easing::EaseOutCubic);
		}
		CreateTimer(0.6, [this]() { Finish(); });
	}

	void BattleStartScene::Finish() {
		if (_finished) return;
		_finished = true;
		if (_onFinished) _onFinished();
	}
}
