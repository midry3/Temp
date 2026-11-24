#include "StageDialogue.h"
#include <Game/Data/SoundEffects.h>

namespace Game {
	void StageDialogue::_Load() {
		// 画面全体のクリックで会話を進める
		Vector2D window = PropertyManager.GetSettingWindowSize();
		if (not (0.0 < window.x) || not (0.0 < window.y)) {
			window = PropertyManager.GetRealWindowSize();
		}
		const auto& blocker = CreateScene<Button>();
		blocker->SetArea({
			Vector2D(0.0, 0.0),
			Vector2D(window.x, 0.0),
			Vector2D(window.x, window.y),
			Vector2D(0.0, window.y)
			});
		const auto& clickSE = CreateSelectSePlayer(blocker.get());
		blocker->LeftPressed += [this, clickSE](const MouseEvent& event) {
			if (not event.just) return;
			clickSE->Play();
			Advance();
			};

		// ダイアログ枠
		const auto& box = CreateScene<ColorRect>();
		box->SetSize({ BOX_WIDTH, BOX_HEIGHT })
			->SetColor(Vector3D(30, 30, 40))
			->SetAlpha(235)
			->SetPosition({ BOX_X, BOX_Y });

		// 左にプレアのアイコン（2倍スケール）
		const auto& icon = CreateScene<Sprite2D>();
		icon->SetTexture(LoadAsset<Texture>("chara/plea/icon.png"))
			->SetScale(ICON_SCALE);
		const auto iconSize = icon->GetRawSize() * ICON_SCALE;
		icon->SetPosition({ BOX_X + ICON_MARGIN, BOX_Y + (BOX_HEIGHT - iconSize.y) * 0.5 });

		// 会話文（アイコンの右側、枠内に収まる幅で表示する）
		const double textX = BOX_X + ICON_MARGIN + iconSize.x + ICON_MARGIN;
		_textLabel = CreateScene<TextLabel>();
		_textLabel->BeginSetting()
			->SetFontSize(TEXT_FONT_SIZE)
			->SetWidth(static_cast<int>(BOX_X + BOX_WIDTH - textX - ICON_MARGIN))
			->EndSetting()
			->SetPosition({ textX, BOX_Y + ICON_MARGIN });

		ShowCurrentLine();
	}

	void StageDialogue::Setup(const std::vector<std::string>& lines) {
		_lines = lines;
		_lineIndex = 0;
		ShowCurrentLine();
	}

	void StageDialogue::ShowCurrentLine() {
		if (not _textLabel) return;
		if (0 <= _lineIndex && _lineIndex < static_cast<int>(_lines.size())) {
			_textLabel->SetText(_lines[_lineIndex]);
		}
	}

	void StageDialogue::Advance() {
		_lineIndex++;
		if (_lineIndex < static_cast<int>(_lines.size())) {
			ShowCurrentLine();
		}
		else {
			Finished.Emit();
		}
	}
}
