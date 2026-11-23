#pragma once
#include <Scene.h>
#include <Scene/Button/Button.h>
#include <Scene/Shape/ColorRect.h>
#include <Scene/Text/TextLabel.h>
#include <Scene/Texture/Sprite2D.h>
#include <functional>
#include <vector>
#include "BoardUnit.h"

namespace Game {
	struct CoopCandidateItem {
		SharedPtr<BoardUnit> unit;
		SharedPtr<Button> button;
		SharedPtr<ColorRect> tileBg;
		SharedPtr<ColorRect> border;
		SharedPtr<Sprite2D> sprite;
		SharedPtr<ColorRect> badgeBg;
		SharedPtr<ColorRect> badgeBorder;
		SharedPtr<TextLabel> badgeLabel;
		SharedPtr<TextLabel> nameLabel;
		bool selected = false;
	};

	struct CoopEncounterMarker {
		SharedPtr<ColorRect> tileBg;
		SharedPtr<ColorRect> border;
		SharedPtr<ColorRect> badgeBg;
		SharedPtr<TextLabel> badgeLabel;
		SharedPtr<Sprite2D> allySprite;
		SharedPtr<Sprite2D> enemySprite;
		SharedPtr<TextLabel> vsLabel;
	};

	class CoopSelectModal : public Scene {
	public:
		using BattleStartCallback = std::function<void(const std::vector<SharedPtr<BoardUnit>>&)>;

	private:
		SharedPtr<ColorRect> _blind;
		SharedPtr<Button> _blindBlocker;

		SharedPtr<ColorRect> _guideBarBg;
		SharedPtr<ColorRect> _guideBarBorder;
		SharedPtr<TextLabel> _guideLabel;

		SharedPtr<Button> _startButton;
		SharedPtr<ColorRect> _startButtonBg;
		SharedPtr<ColorRect> _startButtonBorder;
		SharedPtr<TextLabel> _startButtonLabel;

		CoopEncounterMarker _encounterMarker;
		std::vector<CoopCandidateItem> _candidates;
		BattleStartCallback _onStartBattle;
		double _pulseTime = 0.0;

		void UpdateCandidateVisual(CoopCandidateItem& item);
		void UpdateStartButtonText();

	protected:
		void _Load() override;
		void _Update(float dt) override;

	public:
		void Setup(const SharedPtr<BoardUnit>& primaryAlly, const SharedPtr<BoardUnit>& enemy, const std::vector<SharedPtr<BoardUnit>>& availableAllies, const Vector2D& tileSize, const BattleStartCallback& onStart);
	};
}
