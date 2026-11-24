#pragma once
#include <LogicScene.h>
#include "../../Shader/BlurInShader.h"
#include "../../Shader/ConcentrationLineShader.h"
#include <Scene/Shape/ColorRect.h>
#include <map>
#include <vector>
#include "BattleCharacter/BattleCharacter.h"
#include "SkillInfo.h"
#include <optional>

class EffekseerEmitter2D;
class SoundPlayer;

namespace Game {
	class Pickup;

	namespace {
		class TimeScaleController : public Scene {
		private:
			double _remaining = 0.0;
			float _previousScale = 1.0f;
		protected:
			void _Update(float dt) override;
		public:
			void Start(double duration, double scale);
			void Stop();
		};
	};

	class BattleAttack : public LogicScene {
	private:
		struct Action {
			double time = 0.0;
			std::string type;
			std::string target;
			std::string value;
			std::string effect;
			bool targetOrigin = false;
			std::string sound;
			double volume = 1.0;
			std::vector<std::string> pickupTextures;
			// 演出開始前にあらかじめ用意しておくインスタンス（PreloadAssets で作成）
			SharedPtr<SoundPlayer> soundPlayer;
			int amount = 0;
			int animation = -1;
			bool reverse = false;
			std::optional<bool> loop;
			Vector2D offset;
			Vector2D position;
			bool hasPosition = false;
			bool resetPosition = false;
			bool restorePosition = false;
			std::string easing = "linear";
			Vector2D afterPosition;
			Vector2D afterOffset;
			bool hasAfterPosition = false;
			bool hasAfterOffset = false;
			double afterDuration = 0.0;
			std::string afterEasing = "linear";
			int trigger = -1;
			double triggerTime = 0.0;
			std::vector<std::pair<double, int>> triggers;
			double duration = 0.0;
			double fadeIn = 0.0;
			double fadeOut = 0.0;
			double holdTime = 0.0;
			float flashColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			double scale = 0.5;
			double strength = 2.5;
			double frequency = 30.0;
			double width = 16.0;
			int roundTrips = 3;
			double durationPerTrip = 0.16;
			Vector2D blurCenter;
			double focusRadius = 180.0;
			bool hasBlurCenter = false;
			bool flashUnderEffects = false;
			bool executed = false;
			bool isFinally = false;
			double executeTime = 0.0;
		};
		struct StatChange {
			std::string target = "actor";
			std::string stat;
			int amount = 0;
		};
		SharedPtr<BattleCharacter> _actor;
		SharedPtr<BattleCharacter> _target;
		SkillInfo _skill;
		std::vector<SharedPtr<BattleCharacter>> _targets;
		Vector2D _actorInitialAnimationPosition;
		std::map<SharedPtr<BattleCharacter>, Vector2D> _initialAnimationPositions;
		std::vector<Action> _actions;
		std::vector<StatChange> _statChanges;
		std::vector<SharedPtr<EffekseerEmitter2D>> _effects;
		struct EffectTrigger {
			SharedPtr<EffekseerEmitter2D> effect;
			double time = 0.0;
			int index = 0;
			bool executed = false;
		};
		std::vector<EffectTrigger> _effectTriggers;
		std::vector<SharedPtr<Pickup>> _pickups;
		struct AnimationFinishedConnection {
			SharedPtr<BattleCharacter> target;
			int id = -1;
		};
		std::vector<AnimationFinishedConnection> _animationFinishedConnections;
		double _elapsed = 0.0;
		double _duration = 0.0;
		double _endTime = 0.0;
		bool _mainFinished = false;
		bool _finished = false;
		bool _blurActive = false;
		bool _blurIn = false;
		double _blurElapsed = 0.0;
		double _blurDuration = 0.0;
		int _blurBufferHandle = -1;
		int _blurShaderId = -1;
		BattleBlurShaderParam* _blurBuffer = nullptr;
		bool _concentrationLineActive = false;
		double _concentrationLineElapsed = 0.0;
		double _concentrationLineDuration = 0.0;
		int _concentrationLineBufferHandle = -1;
		int _concentrationLineShaderId = -1;
		BattleConcentrationLineShaderParam* _concentrationLineBuffer = nullptr;
		bool _shakeActive = false;
		double _shakeElapsed = 0.0;
		double _shakeDuration = 0.0;
		double _shakeStrength = 0.0;
		double _shakeFrequency = 30.0;
		bool _flashActive = false;
		double _flashElapsed = 0.0;
		double _flashFadeIn = 0.0;
		double _flashHoldTime = 0.0;
		double _flashFadeOut = 0.0;
		float _flashOpacity = 1.0f;
		SharedPtr<ColorRect> _flashRect;
		SharedPtr<TimeScaleController> _timeScaleController;
		Signal<> _completed;
		Signal<SharedPtr<BattleCharacter>, std::string, int> _statChanged;

		bool LoadTimeline(const std::string& file);
		void PreloadAssets();
		void Execute(Action& action);
		void ApplyStatChanges();
		void StopTimelineEffects();
		void RegisterAnimationFinishedAction(const SharedPtr<BattleCharacter>& target,
			int animation, const Action& action);
		void StopAnimationFinishedConnections();
		void ApplyAfterAnimationPosition(const SharedPtr<BattleCharacter>& target,
			const Action& action);
		void StartPositionTween(const SharedPtr<BattleCharacter>& target, const Vector2D& initialPosition,
			const Action& action);
		void StartBlur(bool blurIn, double duration, double strength,
			const Vector2D& center, double focusRadius, bool hasCenter);
		void UpdateBlur(float dt);
		void StopBlur();
		void StartConcentrationLines(double duration, double strength,
			const Vector2D& center, double focusRadius, bool hasCenter);
		void UpdateConcentrationLines(float dt);
		void StopConcentrationLines();
		void StartShake(double duration, double strength, double frequency);
		void UpdateShake(float dt);
		void StopShake();
		void StartFlash(const float color[4], double fadeIn,
			double holdTime, double fadeOut, bool underEffects);
		void UpdateFlash(float dt);
		void StopFlash();
		void StartTimeScale(double duration, double scale);
		void StopTimeScale();
		void StopEffects();
		void UpdateEffectTriggers();
		void StopPickups();
		SharedPtr<BattleCharacter> ResolveTarget(const std::string& target) const;
		std::vector<SharedPtr<BattleCharacter>> ResolveTargets(const std::string& target) const;
		Vector2D GetInitialAnimationPosition(const SharedPtr<BattleCharacter>& character) const;
	protected:
		void _Update(float dt) override;
	public:
		void Start(const SharedPtr<BattleCharacter>& actor,
			const std::vector<SharedPtr<BattleCharacter>>& targets,
			const SkillInfo& skill);

		// スキルの演出タイムラインが参照するアセットのパスの集合
		struct SkillAssets {
			std::vector<std::string> effects;
			std::vector<std::string> textures;
			std::vector<std::string> sounds;
		};
		// タイムラインを解析して、参照アセットのパスだけを集める（読み込みは行わない）
		static SkillAssets CollectSkillAssets(const SharedPtr<BattleCharacter>& actor, const SkillInfo& skill);
		Signal<>& Completed() { return _completed; }
		Signal<SharedPtr<BattleCharacter>, std::string, int>& StatChanged() { return _statChanged; }
		[[nodiscard]] bool IsFinished() const { return _finished; }
	};
}
