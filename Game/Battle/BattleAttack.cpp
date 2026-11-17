#include "BattleAttack.h"
#include "../../Shader/BlurOutShader.h"
#include <Asset/AssetManager.h>
#include <Effekseer/EffekseerEmitter.h>
#include <Novot.h>
#include <GameState.h>
#include <lua.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace Game {
	namespace {
		class TimeScaleController : public Scene {
		private:
			double _remaining = 0.0;
			float _previousScale = 1.0f;
		protected:
			void _Update(float dt) override {
				_remaining -= dt;
				if (_remaining <= 0.0) {
					GameState.SetTimeScale(_previousScale);
					Disable();
				}
			}
		public:
			void Start(double duration, double scale) {
				_previousScale = GameState.GetTimeScale();
				_remaining = Max(duration, 0.0);
				GameState.SetTimeScale(static_cast<float>(scale));
				if (_remaining <= 0.0) {
					GameState.SetTimeScale(_previousScale);
					Disable();
				}
			}
			void Stop() {
				GameState.SetTimeScale(_previousScale);
				_remaining = 0.0;
				Disable();
			}
		};

		std::string ReadString(lua_State* state, int index, const char* field) {
			lua_getfield(state, index, field);
			const char* value = lua_tostring(state, -1);
			std::string result = value ? value : "";
			lua_pop(state, 1);
			return result;
		}

		double ReadNumber(lua_State* state, int index, const char* field, double fallback = 0.0) {
			lua_getfield(state, index, field);
			const double result = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : fallback;
			lua_pop(state, 1);
			return result;
		}

		Vector2D ReadVector(lua_State* state, int index, const char* field) {
			Vector2D result;
			lua_getfield(state, index, field);
			if (lua_istable(state, -1)) {
				result.x = ReadNumber(state, -1, "x");
				result.y = ReadNumber(state, -1, "y");
			}
			lua_pop(state, 1);
			return result;
		}

		bool HasTable(lua_State* state, int index, const char* field) {
			lua_getfield(state, index, field);
			const bool result = lua_istable(state, -1);
			lua_pop(state, 1);
			return result;
		}
	}

	void BattleAttack::Start(const SharedPtr<BattleCharacter>& actor,
		const SharedPtr<BattleCharacter>& target, const SkillInfo& skill) {
		SetName("BattleAttack");
		StopEffects();
		for (const auto& effect : _effects) {
			DeleteScene(effect);
		}
		_actor = actor;
		_target = target;
		_actions.clear();
		_effects.clear();
		StopBlur();
		StopConcentrationLines();
		StopShake();
		StopTimeScale();
		_elapsed = 0.0;
		_duration = 0.0;
		_finished = false;
		LoadTimeline(skill.directionFile);
		Enable();
	}

	void BattleAttack::StopEffects() {
		for (const auto& effect : _effects) {
			if (effect) {
				effect->Stop();
				effect->Disable();
			}
		}
	}

	bool BattleAttack::LoadTimeline(const std::string& file) {
		lua_State* state = luaL_newstate();
		if (!state) return false;
		luaL_openlibs(state);
		const auto resolvedFile = AssetManager.ResolvePath(file);
		if (luaL_dofile(state, resolvedFile.string().c_str()) != LUA_OK) {
			std::cerr << "Battle attack Lua load error (" << file << "): "
				<< lua_tostring(state, -1) << std::endl;
			lua_close(state);
			return false;
		}

		lua_getglobal(state, "attack");
		if (not lua_istable(state, -1)) {
			lua_pop(state, 1);
			lua_close(state);
			return false;
		}
		_duration = ReadNumber(state, -1, "duration", 0.1);
		lua_getfield(state, -1, "actions");
		if (lua_istable(state, -1)) {
			const auto count = static_cast<int>(lua_rawlen(state, -1));
			for (int i = 1; i <= count; ++i) {
				lua_rawgeti(state, -1, i);
				if (lua_istable(state, -1)) {
					Action action;
					action.time = ReadNumber(state, -1, "time");
					action.type = ReadString(state, -1, "type");
					action.target = ReadString(state, -1, "target");
					action.value = ReadString(state, -1, "value");
					action.effect = ReadString(state, -1, "effect");
					action.amount = static_cast<int>(ReadNumber(state, -1, "amount"));
					action.animation = static_cast<int>(ReadNumber(state, -1, "animation", -1));
					action.offset = ReadVector(state, -1, "offset");
					action.duration = ReadNumber(state, -1, "duration");
					action.scale = ReadNumber(state, -1, "scale", 0.5);
					action.strength = ReadNumber(state, -1, "strength", 2.5);
					action.frequency = ReadNumber(state, -1, "frequency", 30.0);
					action.blurCenter = ReadVector(state, -1, "center");
					action.hasBlurCenter = HasTable(state, -1, "center");
					action.focusRadius = ReadNumber(state, -1, "focusRadius", 360.0);
					_actions.push_back(action);
				}
				lua_pop(state, 1);
			}
		}
		lua_close(state);
		std::sort(_actions.begin(), _actions.end(),
			[](const Action& left, const Action& right) { return left.time < right.time; });
		return true;
	}

	SharedPtr<BattleCharacter> BattleAttack::ResolveTarget(const std::string& target) const {
		if (target == "actor") return _actor;
		if (target == "target") return _target;
		return nullptr;
	}

	void BattleAttack::Execute(Action& action) {
		if (action.type == "damage") {
			if (auto target = ResolveTarget(action.target)) target->ApplyDamage(action.amount);
		}
		else if (action.type == "animation") {
			if (auto target = ResolveTarget(action.target)) target->PlayAnimation(action.animation);
		}
		else if (action.type == "effect" && not action.effect.empty()) {
			auto effect = CreateScene<EffekseerEmitter2D>();
			effect->SetEffect(action.effect);
			if (_actor) effect->SetGlobalPosition(_actor->GetGlobalPosition() + action.offset);
			effect->Play();
			_effects.push_back(effect);
		}
		else if (action.type == "blur_in") {
			StartBlur(true, action.duration, action.strength, action.blurCenter,
				action.focusRadius, action.hasBlurCenter);
		}
		else if (action.type == "blur_out") {
			StartBlur(false, action.duration, action.strength, action.blurCenter,
				action.focusRadius, action.hasBlurCenter);
		}
		else if (action.type == "concentration_lines") {
			StartConcentrationLines(action.duration, action.strength, action.blurCenter,
				action.focusRadius, action.hasBlurCenter);
		}
		else if (action.type == "screen_shake") {
			StartShake(action.duration, action.strength, action.frequency);
		}
		else if (action.type == "time_scale") {
			StartTimeScale(action.duration, action.scale);
		}
	}

	void BattleAttack::_Update(float dt) {
		if (_finished) return;
		_elapsed += dt;
		UpdateBlur(dt);
		UpdateConcentrationLines(dt);
		UpdateShake(dt);
		for (auto& action : _actions) {
			if (not action.executed && _elapsed >= action.time) {
				action.executed = true;
				Execute(action);
			}
		}
		if (_elapsed >= _duration) {
			_finished = true;
			StopEffects();
			StopBlur();
			StopConcentrationLines();
			StopShake();
			StopTimeScale();
			_completed.Emit();
		}
	}

	void BattleAttack::StartBlur(bool blurIn, double duration, double strength,
		const Vector2D& center, double focusRadius, bool hasCenter) {
		StopBlur();
		_blurIn = blurIn;
		_blurElapsed = 0.0;
		_blurDuration = Max(duration, 0.0);
		_blurActive = true;
		const auto windowSize = PropertyManager.GetSettingWindowSize();
		_blurBufferHandle = CoreBackend::Shader::CreateBuffer(sizeof(BattleBlurShaderParam));
		_blurBuffer = static_cast<BattleBlurShaderParam*>(
			CoreBackend::Shader::Buffer(_blurBufferHandle));
		_blurBuffer->progress = blurIn ? 0.0f : 1.0f;
		_blurBuffer->strength = static_cast<float>(Clamp(strength, 0.0, 8.0));
		_blurBuffer->texelSize[0] = 1.0f / static_cast<float>(windowSize.x);
		_blurBuffer->texelSize[1] = 1.0f / static_cast<float>(windowSize.y);
		_blurBuffer->focusCenter[0] = hasCenter
			? static_cast<float>(center.x / windowSize.x) : 0.5f;
		_blurBuffer->focusCenter[1] = hasCenter
			? static_cast<float>(center.y / windowSize.y) : 0.5f;
		_blurBuffer->focusRadius = static_cast<float>(Clamp(focusRadius, 0.0, 2000.0));
		_blurBuffer->padding = 0.0f;
		auto blurShader = blurIn
			? std::shared_ptr<PixelShader>(std::make_shared<BlurInShader>())
			: std::shared_ptr<PixelShader>(std::make_shared<BlurOutShader>());
		_blurShaderId = Novot::AddScreenPixelShader(blurShader);
		Novot::SetConstantBufferToScreenPixelShader(
			_blurShaderId, _blurBufferHandle, NOVOT_CONSTANT_BUFFER_SLOT_START);
		CoreBackend::Shader::UpdateConstantBuffer(_blurBufferHandle);
		if (_blurDuration <= 0.0) {
			UpdateBlur(0.0f);
		}
	}

	void BattleAttack::UpdateBlur(float dt) {
		if (not _blurActive or _blurBuffer == nullptr) return;
		_blurElapsed += dt;
		const auto ratio = _blurDuration <= 0.0
			? 1.0
			: Clamp(_blurElapsed / _blurDuration, 0.0, 1.0);
		_blurBuffer->progress = static_cast<float>(_blurIn ? ratio : 1.0 - ratio);
		if (ratio >= 1.0) {
			_blurActive = false;
			if (not _blurIn) {
				Novot::RemoveScreenPixelShader(_blurShaderId);
				_blurShaderId = -1;
			}
		}
	}

	void BattleAttack::StopBlur() {
		if (_blurShaderId != -1) {
			Novot::RemoveScreenPixelShader(_blurShaderId);
		}
		_blurActive = false;
		_blurBuffer = nullptr;
		_blurBufferHandle = -1;
		_blurShaderId = -1;
	}

	void BattleAttack::StartConcentrationLines(double duration, double strength,
		const Vector2D& center, double focusRadius, bool hasCenter) {
		StopConcentrationLines();
		_concentrationLineElapsed = 0.0;
		_concentrationLineDuration = Max(duration, 0.0);
		_concentrationLineActive = true;
		const auto windowSize = PropertyManager.GetSettingWindowSize();
		_concentrationLineBufferHandle = CoreBackend::Shader::CreateBuffer(
			sizeof(BattleConcentrationLineShaderParam));
		_concentrationLineBuffer = static_cast<BattleConcentrationLineShaderParam*>(
			CoreBackend::Shader::Buffer(_concentrationLineBufferHandle));
		_concentrationLineBuffer->progress = 1.0f;
		_concentrationLineBuffer->strength = static_cast<float>(Clamp(strength, 0.0, 8.0));
		_concentrationLineBuffer->texelSize[0] =
			1.0f / static_cast<float>(windowSize.x);
		_concentrationLineBuffer->texelSize[1] =
			1.0f / static_cast<float>(windowSize.y);
		_concentrationLineBuffer->focusCenter[0] = hasCenter
			? static_cast<float>(center.x / windowSize.x) : 0.5f;
		_concentrationLineBuffer->focusCenter[1] = hasCenter
			? static_cast<float>(center.y / windowSize.y) : 0.5f;
		_concentrationLineBuffer->focusRadius =
			static_cast<float>(Clamp(focusRadius, 0.0, 2000.0));
		_concentrationLineBuffer->rayCount = 36.0f;
		_concentrationLineBuffer->time = 0.0f;
		_concentrationLineBuffer->padding = 0.0f;
		_concentrationLineShaderId = Novot::AddScreenPixelShader(
			std::make_shared<ConcentrationLineShader>());
		Novot::SetConstantBufferToScreenPixelShader(
			_concentrationLineShaderId,
			_concentrationLineBufferHandle,
			NOVOT_CONSTANT_BUFFER_SLOT_START);
		CoreBackend::Shader::UpdateConstantBuffer(_concentrationLineBufferHandle);
		if (_concentrationLineDuration <= 0.0) {
			UpdateConcentrationLines(0.0f);
		}
	}

	void BattleAttack::UpdateConcentrationLines(float dt) {
		if (not _concentrationLineActive or _concentrationLineBuffer == nullptr) return;
		_concentrationLineElapsed += dt;
		const auto progress = _concentrationLineDuration <= 0.0
			? 0.0
			: Clamp(1.0 - _concentrationLineElapsed / _concentrationLineDuration, 0.0, 1.0);
		_concentrationLineBuffer->progress = static_cast<float>(progress);
		_concentrationLineBuffer->time += dt;
		if (progress >= 1.0) {
			_concentrationLineActive = false;
			Novot::RemoveScreenPixelShader(_concentrationLineShaderId);
			_concentrationLineShaderId = -1;
		}
	}

	void BattleAttack::StopConcentrationLines() {
		if (_concentrationLineShaderId != -1) {
			Novot::RemoveScreenPixelShader(_concentrationLineShaderId);
		}
		_concentrationLineActive = false;
		_concentrationLineBuffer = nullptr;
		_concentrationLineBufferHandle = -1;
		_concentrationLineShaderId = -1;
	}

	void BattleAttack::StartShake(double duration, double strength, double frequency) {
		StopShake();
		_shakeActive = true;
		_shakeElapsed = 0.0;
		_shakeDuration = Max(duration, 0.0);
		_shakeStrength = Max(strength, 0.0);
		_shakeFrequency = Max(frequency, 0.0);
		Novot::SetScreenShake(Vector2D(0, 0));
		if (_shakeDuration <= 0.0) {
			UpdateShake(0.0f);
		}
	}

	void BattleAttack::UpdateShake(float dt) {
		if (not _shakeActive) return;
		_shakeElapsed += dt;
		const auto ratio = _shakeDuration <= 0.0
			? 1.0
			: Clamp(_shakeElapsed / _shakeDuration, 0.0, 1.0);
		const auto fade = 1.0 - ratio;
		const auto phase = _shakeElapsed * _shakeFrequency * 2.0 * PI;
		const auto x = std::sin(phase) * _shakeStrength * fade;
		const auto y = std::cos(phase * 1.17) * _shakeStrength * fade;
		Novot::SetScreenShake(Vector2D(x, y));
		if (ratio >= 1.0) {
			StopShake();
		}
	}

	void BattleAttack::StopShake() {
		if (_shakeActive) {
			Novot::ClearScreenShake();
		}
		_shakeActive = false;
		_shakeElapsed = 0.0;
		_shakeDuration = 0.0;
	}

	void BattleAttack::StartTimeScale(double duration, double scale) {
		StopTimeScale();
		auto controller = CreateScene<TimeScaleController>();
		controller->SetIgnoreTimeScale(true);
		controller->Start(duration, scale);
		_timeScaleController = controller;
	}

	void BattleAttack::StopTimeScale() {
		if (auto controller = std::dynamic_pointer_cast<TimeScaleController>(_timeScaleController)) {
			controller->Stop();
		}
		_timeScaleController.reset();
	}
}
