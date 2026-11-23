#include "CoopSelectModal.h"
#include <Asset/AssetManager.h>
#include <cmath>

namespace Game {
	void CoopSelectModal::_Load() {
		SetUIMode(true);

		// 盤面全体を薄暗く覆う黒ブラインド
		_blind = CreateScene<ColorRect>();
		_blind->SetSize({ 2560.0, 1440.0 })->SetColor(Vector3D(0, 0, 0))->SetAlpha(165);
		_blind->SetPosition({ -200.0, -200.0 });

		// 盤面背面への誤クリックを遮断
		_blindBlocker = CreateScene<Button>();
		_blindBlocker->SetArea({
			Vector2D(-200.0, -200.0),
			Vector2D(2360.0, -200.0),
			Vector2D(2360.0, 1240.0),
			Vector2D(-200.0, 1240.0)
		});

		// 画面上部ガイドバー
		_guideBarBorder = CreateScene<ColorRect>();
		_guideBarBorder->SetSize({ 584.0, 44.0 })->SetColor(Vector3D(70, 120, 200))->SetAlpha(200);
		_guideBarBorder->SetPosition({ 348.0, 14.0 });

		_guideBarBg = CreateScene<ColorRect>();
		_guideBarBg->SetSize({ 580.0, 40.0 })->SetColor(Vector3D(20, 26, 42))->SetAlpha(235);
		_guideBarBg->SetPosition({ 350.0, 16.0 });

		_guideLabel = CreateScene<TextLabel>();
		_guideLabel->SetFontSize(16)->SetForegroundColor(CoreBackend::Drawing::Color(220, 235, 255));
		_guideLabel->SetText("参戦する仲間の選択")->SetPosition({ 380.0, 24.0 });

		// 画面下部 戦闘開始ボタン
		_startButton = CreateScene<Button>();
		_startButton->SetArea({
			Vector2D(0.0, 0.0),
			Vector2D(200.0, 0.0),
			Vector2D(200.0, 80.0),
			Vector2D(0.0, 80.0)
		});
		_startButton->SetPosition({ 510.0, 400.0 });

		_startButtonBorder = CreateScene<ColorRect>();
		_startButtonBorder->SetSize({ 264.0, 52.0 })->SetColor(Vector3D(255, 200, 80))->SetAlpha(220);
		_startButtonBorder->SetPosition({ 508.0, 648.0 });

		_startButtonBg = CreateScene<ColorRect>();
		_startButtonBg->SetSize({ 260.0, 48.0 })->SetColor(Vector3D(180, 50, 40))->SetAlpha(240);
		_startButtonBg->SetPosition({ 510.0, 650.0 });

		_startButtonLabel = CreateScene<TextLabel>();
		_startButtonLabel->SetFontSize(18)->SetForegroundColor(CoreBackend::Drawing::Color(255, 255, 255));
		_startButtonLabel->SetText("戦闘開始 (単独)")->SetPosition({ 570.0, 400.0 });

		_startButton->Entered += [this](const MouseEvent&) {
			_startButtonBg->SetColor(Vector3D(220, 70, 60));
		};
		_startButton->Exited += [this](const MouseEvent&) {
			_startButtonBg->SetColor(Vector3D(180, 50, 40));
		};
		_startButton->LeftPressed += [this](const MouseEvent& event) {
			if (not event.just) return;
			std::vector<SharedPtr<BoardUnit>> selectedAllies;
			for (const auto& item : _candidates) {
				if (item.selected && item.unit) {
					selectedAllies.push_back(item.unit);
				}
			}
			if (_onStartBattle) {
				_onStartBattle(selectedAllies);
			}
		};
	}

	void CoopSelectModal::Setup(const SharedPtr<BoardUnit>& primaryAlly, const SharedPtr<BoardUnit>& enemy, const std::vector<SharedPtr<BoardUnit>>& availableAllies, const Vector2D& tileSize, const BattleStartCallback& onStart) {
		_onStartBattle = onStart;

		// 1. 交戦マスをブラインドの上にハイライト表示
		const Vector2D battlePos = enemy ? enemy->GetPosition() : (primaryAlly ? primaryAlly->GetPosition() : Vec2D_0);

		_encounterMarker.border = CreateScene<ColorRect>();
		_encounterMarker.border->SetSize({ tileSize.x + 8.0, tileSize.y + 8.0 });
		_encounterMarker.border->SetColor(Vector3D(255, 70, 70))->SetAlpha(240);
		_encounterMarker.border->SetPosition({ battlePos.x - 4.0, battlePos.y - 4.0 });

		_encounterMarker.tileBg = CreateScene<ColorRect>();
		_encounterMarker.tileBg->SetSize(tileSize);
		_encounterMarker.tileBg->SetColor(Vector3D(60, 20, 28))->SetAlpha(220);
		_encounterMarker.tileBg->SetPosition(battlePos);

		_encounterMarker.badgeBg = CreateScene<ColorRect>();
		_encounterMarker.badgeBg->SetSize({ tileSize.x + 16.0, 20.0 });
		_encounterMarker.badgeBg->SetColor(Vector3D(180, 30, 30))->SetAlpha(240);
		_encounterMarker.badgeBg->SetPosition({ battlePos.x - 8.0, battlePos.y - 24.0 });

		_encounterMarker.badgeLabel = CreateScene<TextLabel>();
		_encounterMarker.badgeLabel->SetFontSize(12)->SetForegroundColor(CoreBackend::Drawing::Color(255, 230, 230));
		_encounterMarker.badgeLabel->SetText("【交戦マス】")->SetPosition({ battlePos.x - 4.0, battlePos.y - 22.0 });

		// 交戦マス内のアバター表示（左: 先頭味方, 右: 敵）
		if (primaryAlly) {
			const auto& data = primaryAlly->GetData();
			if (not data.texture.empty()) {
				_encounterMarker.allySprite = CreateScene<Sprite2D>();
				_encounterMarker.allySprite->SetTexture(LoadAsset<Texture>(data.texture));
				const auto rawSize = _encounterMarker.allySprite->GetRawSize();
				if (0.0 < rawSize.x && 0.0 < rawSize.y) {
					const double targetW = tileSize.x * 0.44;
					_encounterMarker.allySprite->SetScale(Min(targetW / rawSize.x, tileSize.y * 0.7 / rawSize.y));
				}
				_encounterMarker.allySprite->SetPosition({ battlePos.x + 2.0, battlePos.y + tileSize.y * 0.15 });
			}
		}

		_encounterMarker.vsLabel = CreateScene<TextLabel>();
		_encounterMarker.vsLabel->SetFontSize(13)->SetForegroundColor(CoreBackend::Drawing::Color(255, 80, 80));
		_encounterMarker.vsLabel->SetText("VS")->SetPosition({ battlePos.x + tileSize.x * 0.42, battlePos.y + tileSize.y * 0.35 });

		if (enemy) {
			const auto& data = enemy->GetData();
			if (not data.texture.empty()) {
				_encounterMarker.enemySprite = CreateScene<Sprite2D>();
				_encounterMarker.enemySprite->SetTexture(LoadAsset<Texture>(data.texture));
				const auto rawSize = _encounterMarker.enemySprite->GetRawSize();
				if (0.0 < rawSize.x && 0.0 < rawSize.y) {
					const double targetW = tileSize.x * 0.44;
					_encounterMarker.enemySprite->SetScale(Min(targetW / rawSize.x, tileSize.y * 0.7 / rawSize.y));
				}
				_encounterMarker.enemySprite->SetPosition({ battlePos.x + tileSize.x * 0.54, battlePos.y + tileSize.y * 0.15 });
			}
		}

		// 2. 共闘候補ユニットをブラインドの上にハイライト表示し、マップ上で直接選択可能にする
		_candidates.clear();
		const int count = static_cast<int>(availableAllies.size());

		for (int i = 0; i < count; i++) {
			const auto& ally = availableAllies[i];
			const Vector2D pos = ally->GetPosition();

			CoopCandidateItem item;
			item.unit = ally;
			item.selected = false;

			// 外枠（選択時にゴールドでパルス発光）
			item.border = CreateScene<ColorRect>();
			item.border->SetSize({ tileSize.x + 8.0, tileSize.y + 8.0 });
			item.border->SetColor(Vector3D(80, 200, 255))->SetAlpha(220);
			item.border->SetPosition({ pos.x - 4.0, pos.y - 4.0 });

			// マス背景
			item.tileBg = CreateScene<ColorRect>();
			item.tileBg->SetSize(tileSize);
			item.tileBg->SetColor(Vector3D(20, 40, 55))->SetAlpha(200);
			item.tileBg->SetPosition(pos);

			// キャラクターアイコン（盤面と同じ位置・サイズ）
			item.sprite = CreateScene<Sprite2D>();
			const auto& data = ally->GetData();
			if (not data.texture.empty()) {
				item.sprite->SetTexture(LoadAsset<Texture>(data.texture));
				const auto rawSize = item.sprite->GetRawSize();
				if (0.0 < rawSize.x && 0.0 < rawSize.y) {
					const auto scale = Min(tileSize.x / rawSize.x, tileSize.y / rawSize.y) * 0.8;
					item.sprite->SetScale(scale);
					item.sprite->SetPosition(pos + tileSize * 0.1);
				}
			}

			// 頭上バッジ枠 & 背景
			item.badgeBorder = CreateScene<ColorRect>();
			item.badgeBorder->SetSize({ tileSize.x + 16.0, 22.0 });
			item.badgeBorder->SetColor(Vector3D(70, 180, 240))->SetAlpha(220);
			item.badgeBorder->SetPosition({ pos.x - 8.0, pos.y - 25.0 });

			item.badgeBg = CreateScene<ColorRect>();
			item.badgeBg->SetSize({ tileSize.x + 14.0, 20.0 });
			item.badgeBg->SetColor(Vector3D(20, 32, 48))->SetAlpha(240);
			item.badgeBg->SetPosition({ pos.x - 7.0, pos.y - 24.0 });

			item.badgeLabel = CreateScene<TextLabel>();
			item.badgeLabel->SetFontSize(12)->SetForegroundColor(CoreBackend::Drawing::Color(140, 220, 255));
			item.badgeLabel->SetText("参戦可")->SetPosition({ pos.x + 2.0, pos.y - 22.0 });

			// キャラクター名ラベル（足元）
			item.nameLabel = CreateScene<TextLabel>();
			item.nameLabel->SetFontSize(12)->SetForegroundColor(CoreBackend::Drawing::Color(240, 240, 240));
			item.nameLabel->SetText(data.displayName)->SetPosition({ pos.x + 4.0, pos.y + tileSize.y * 0.76 });

			// クリック判定ボタン（マスの位置に配置）
			item.button = CreateScene<Button>();
			item.button->SetArea({
				Vector2D(0.0, 0.0),
				Vector2D(tileSize.x, 0.0),
				tileSize,
				Vector2D(0.0, tileSize.y)
			});
			item.button->SetPosition(pos);

			const int candidateIndex = i;
			item.button->LeftPressed += [this, candidateIndex](const MouseEvent& event) {
				if (not event.just) return;
				if (candidateIndex < 0 || not (candidateIndex < static_cast<int>(_candidates.size()))) return;
				auto& targetItem = _candidates[candidateIndex];
				targetItem.selected = not targetItem.selected;
				UpdateCandidateVisual(targetItem);
				UpdateStartButtonText();
			};

			_candidates.push_back(item);
			UpdateCandidateVisual(_candidates.back());
		}

		UpdateStartButtonText();
	}

	void CoopSelectModal::UpdateCandidateVisual(CoopCandidateItem& item) {
		if (item.selected) {
			item.tileBg->SetColor(Vector3D(35, 80, 45))->SetAlpha(220);
			item.border->SetColor(Vector3D(255, 215, 60))->SetAlpha(255);
			item.badgeBorder->SetColor(Vector3D(255, 215, 60))->SetAlpha(255);
			item.badgeBg->SetColor(Vector3D(25, 75, 35))->SetAlpha(250);
			item.badgeLabel->SetForegroundColor(CoreBackend::Drawing::Color(120, 255, 150));
			item.badgeLabel->SetText("★ 参戦中!");
		}
		else {
			item.tileBg->SetColor(Vector3D(20, 40, 55))->SetAlpha(200);
			item.border->SetColor(Vector3D(80, 200, 255))->SetAlpha(220);
			item.badgeBorder->SetColor(Vector3D(70, 180, 240))->SetAlpha(220);
			item.badgeBg->SetColor(Vector3D(20, 32, 48))->SetAlpha(240);
			item.badgeLabel->SetForegroundColor(CoreBackend::Drawing::Color(140, 220, 255));
			item.badgeLabel->SetText("参戦可");
		}
	}

	void CoopSelectModal::UpdateStartButtonText() {
		int selectedCount = 0;
		for (const auto& item : _candidates) {
			if (item.selected) {
				selectedCount++;
			}
		}
		if (selectedCount == 0) {
			_startButtonLabel->SetText("戦闘開始 (単独)");
			_startButtonLabel->SetPosition({ 570.0, 662.0 });
		}
		else {
			_startButtonLabel->SetText("戦闘開始 (共闘 " + std::to_string(selectedCount) + "人)");
			_startButtonLabel->SetPosition({ 550.0, 662.0 });
		}
	}

	void CoopSelectModal::_Update(float dt) {
		Scene::_Update(dt);
		_pulseTime += dt;
		const auto pulse = (std::sin(_pulseTime * 5.5) + 1.0) * 0.5;

		// 交戦マスのパルス
		if (_encounterMarker.border) {
			const Vector3D red(255, 60, 60);
			const Vector3D orange(255, 160, 80);
			_encounterMarker.border->SetColor(red * (1.0 - pulse) + orange * pulse);
		}

		// 候補ユニットのパルス
		for (auto& item : _candidates) {
			if (item.selected) {
				if (item.border) {
					const Vector3D gold(255, 215, 60);
					const Vector3D bright(255, 255, 180);
					item.border->SetColor(gold * (1.0 - pulse) + bright * pulse);
				}
				if (item.badgeBorder) {
					const Vector3D gold(255, 215, 60);
					const Vector3D bright(255, 255, 180);
					item.badgeBorder->SetColor(gold * (1.0 - pulse) + bright * pulse);
				}
			}
			else {
				if (item.border) {
					const Vector3D cyan(60, 180, 240);
					const Vector3D lightCyan(140, 230, 255);
					item.border->SetColor(cyan * (1.0 - pulse) + lightCyan * pulse);
				}
			}
		}
	}
}
