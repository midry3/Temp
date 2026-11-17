#pragma once
#include <LogicScene.h>
#include "../../Shader/BlurInShader.h"
#include "../../Shader/ConcentrationLineShader.h"
#include <vector>
#include "BattleCharacter/BattleCharacter.h"
#include "SkillInfo.h"

class EffekseerEmitter2D;

namespace Game {
	class BattleAttack : public LogicScene {
	private:
		struct Action {
			double time = 0.0;
			std::string type;
			std::string target;
			std::string value;
			std::string effect;
			int amount = 0;
			int animation = -1;
			Vector2D offset;
			double duration = 0.0;
			double scale = 0.5;
			double strength = 2.5;
			double frequency = 30.0;
			Vector2D blurCenter;
			double focusRadius = 180.0;
			bool hasBlurCenter = false;
			bool executed = false;
		};
		SharedPtr<BattleCharacter> _actor;
		SharedPtr<BattleCharacter> _target;
		std::vector<Action> _actions;
		std::vector<SharedPtr<EffekseerEmitter2D>> _effects;
		double _elapsed = 0.0;
		double _duration = 0.0;
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
		SharedPtr<Scene> _timeScaleController;
		Signal<> _completed;

		bool LoadTimeline(const std::string& file);
		void Execute(Action& action);
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
		void StartTimeScale(double duration, double scale);
		void StopTimeScale();
		void StopEffects();
		SharedPtr<BattleCharacter> ResolveTarget(const std::string& target) const;
	protected:
		void _Update(float dt) override;
	public:
		void Start(const SharedPtr<BattleCharacter>& actor,
			const SharedPtr<BattleCharacter>& target,
			const SkillInfo& skill);
		Signal<>& Completed() { return _completed; }
		[[nodiscard]] bool IsFinished() const { return _finished; }
	};
}
