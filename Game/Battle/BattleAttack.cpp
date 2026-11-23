#include "BattleAttack.h"
#include "../../Shader/BlurOutShader.h"
#include "../Goods/Pickup.h"
#include <Asset/AssetManager.h>
#include <Effekseer/EffekseerEmitter.h>
#include <Animation/Tween.h>
#include <Animation/EasingFunction.h>
#include <Math/Utils.h>
#include <Novot.h>
#include <GameState.h>
#include <lua.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <initializer_list>

namespace Game {
	namespace {
		void TimeScaleController::_Update(float dt) {
			_remaining -= dt;
			if (_remaining <= 0.0) {
				GameState.SetTimeScale(_previousScale);
				Disable();
			}
		}

		void TimeScaleController::Start(double duration, double scale) {
			_previousScale = GameState.GetTimeScale();
			_remaining = Max(duration, 0.0);
			GameState.SetTimeScale(static_cast<float>(scale));
			if (_remaining <= 0.0) {
				GameState.SetTimeScale(_previousScale);
				Disable();
			}
		}

		void TimeScaleController::Stop() {
			GameState.SetTimeScale(_previousScale);
			_remaining = 0.0;
			Disable();
		}

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

		bool ReadBool(lua_State* state, int index, const char* field, bool fallback = false) {
			lua_getfield(state, index, field);
			const bool result = lua_isboolean(state, -1)
				? lua_toboolean(state, -1) != 0 : fallback;
			lua_pop(state, 1);
			return result;
		}

		std::optional<bool> ReadOptionalBool(lua_State* state, int index, const char* field) {
			lua_getfield(state, index, field);
			std::optional<bool> result;
			if (lua_isboolean(state, -1)) {
				result = lua_toboolean(state, -1) != 0;
			}
			lua_pop(state, 1);
			return result;
		}

		std::vector<std::string> ReadStringArray(
			lua_State* state, int index, const char* field) {
			std::vector<std::string> result;
			lua_getfield(state, index, field);
			if (lua_istable(state, -1)) {
				const auto count = static_cast<int>(lua_rawlen(state, -1));
				for (int i = 1; i <= count; ++i) {
					lua_rawgeti(state, -1, i);
					if (const char* value = lua_tostring(state, -1)) {
						result.emplace_back(value);
					}
					lua_pop(state, 1);
				}
			}
			lua_pop(state, 1);
			return result;
		}

		std::vector<std::pair<double, int>> ReadEffectTriggers(
			lua_State* state, int index) {
			std::vector<std::pair<double, int>> result;
			lua_getfield(state, index, "triggers");
			if (lua_istable(state, -1)) {
				const auto count = static_cast<int>(lua_rawlen(state, -1));
				for (int i = 1; i <= count; ++i) {
					lua_rawgeti(state, -1, i);
					if (lua_istable(state, -1)) {
						const double time = ReadNumber(state, -1, "time");
						const int trigger = static_cast<int>(ReadNumber(state, -1, "trigger", -1));
						if (trigger >= 0) result.emplace_back(time, trigger);
					}
					lua_pop(state, 1);
				}
			}
			lua_pop(state, 1);
			return result;
		}

		double ReadFirstNumber(lua_State* state, int index,
			std::initializer_list<const char*> fields, double fallback = 0.0) {
			for (const char* field : fields) {
				lua_getfield(state, index, field);
				if (lua_isnumber(state, -1)) {
					const double result = lua_tonumber(state, -1);
					lua_pop(state, 1);
					return result;
				}
				lua_pop(state, 1);
			}
			return fallback;
		}

		bool ReadColor(lua_State* state, int index,
			const char* field, float result[4]) {
			lua_getfield(state, index, field);
			if (not lua_istable(state, -1)) {
				lua_pop(state, 1);
				return false;
			}
			auto readChannel = [state](int tableIndex, const char* channel, float fallback) {
				lua_getfield(state, tableIndex, channel);
				const float value = lua_isnumber(state, -1)
					? static_cast<float>(lua_tonumber(state, -1)) : fallback;
				lua_pop(state, 1);
				return value;
				};
			result[0] = readChannel(lua_gettop(state), "r", 255.0f);
			result[1] = readChannel(lua_gettop(state), "g", 255.0f);
			result[2] = readChannel(lua_gettop(state), "b", 255.0f);
			result[3] = readChannel(lua_gettop(state), "a", 255.0f);
			lua_pop(state, 1);

			const bool normalized = result[0] <= 1.0f
				&& result[1] <= 1.0f
				&& result[2] <= 1.0f
				&& result[3] <= 1.0f;
			if (not normalized) {
				result[0] /= 255.0f;
				result[1] /= 255.0f;
				result[2] /= 255.0f;
				result[3] /= 255.0f;
			}
			for (int i = 0; i < 4; ++i) {
				result[i] = Clamp(result[i], 0.0f, 1.0f);
			}
			return true;
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
		StopAnimationFinishedConnections();
		_actorInitialAnimationPosition = _actor
			? *(_actor->GetAnimationPositionPointer()) : Vector2D();
		_targetInitialAnimationPosition = _target
			? *(_target->GetAnimationPositionPointer()) : Vector2D();
		_actions.clear();
		_effects.clear();
		_effectTriggers.clear();
		StopPickups();
		StopBlur();
		StopConcentrationLines();
		StopShake();
		StopTimeScale();
		StopFlash();
		_elapsed = 0.0;
		_duration = 0.0;
		_endTime = 0.0;
		_mainFinished = false;
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
		_effectTriggers.clear();
	}

	void BattleAttack::UpdateEffectTriggers() {
		for (auto& trigger : _effectTriggers) {
			if (trigger.executed || _elapsed < trigger.time) continue;
			if (trigger.effect) {
				trigger.effect->SendTrigger(trigger.index);
			}
			trigger.executed = true;
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
					action.isFinally = ReadBool(state, -1, "finally");
					action.executeTime = action.isFinally
						? _duration + Max(action.time, 0.0) : action.time;
					action.type = ReadString(state, -1, "type");
					action.target = ReadString(state, -1, "target");
					action.value = ReadString(state, -1, "value");
					action.effect = ReadString(state, -1, "effect");
					action.pickupTextures = ReadStringArray(state, -1, "textures");
					action.trigger = static_cast<int>(ReadNumber(state, -1, "trigger", -1));
					action.triggerTime = ReadNumber(state, -1, "triggerTime");
					action.triggers = ReadEffectTriggers(state, -1);
					action.amount = static_cast<int>(ReadNumber(state, -1, "amount"));
					action.animation = static_cast<int>(ReadNumber(state, -1, "animation", -1));
					action.reverse = ReadBool(state, -1, "reverse");
					action.loop = ReadOptionalBool(state, -1, "loop");
					action.offset = ReadVector(state, -1, "offset");
					action.position = ReadVector(state, -1, "position");
					action.hasPosition = HasTable(state, -1, "position");
					action.resetPosition = ReadBool(state, -1, "reset");
					action.restorePosition = ReadBool(state, -1, "restore");
					action.easing = ReadString(state, -1, "easing");
					if (action.easing.empty()) action.easing = "linear";
					lua_getfield(state, -1, "after");
					if (lua_istable(state, -1)) {
						action.afterPosition = ReadVector(state, -1, "position");
						action.afterOffset = ReadVector(state, -1, "offset");
						action.hasAfterPosition = HasTable(state, -1, "position");
						action.hasAfterOffset = HasTable(state, -1, "offset");
						action.afterDuration = ReadNumber(state, -1, "duration");
						action.afterEasing = ReadString(state, -1, "easing");
						if (action.afterEasing.empty()) action.afterEasing = "linear";
					}
					lua_pop(state, 1);
					action.duration = ReadNumber(state, -1, "duration");
					action.fadeIn = ReadFirstNumber(state, -1,
						{ "fadeIn", "fadein", "fade_in" });
					action.fadeOut = ReadFirstNumber(state, -1,
						{ "fadeOut", "fadeout", "fade_out" });
					action.flashUnderEffects = ReadBool(state, -1, "underEffects");
					action.holdTime = ReadFirstNumber(state, -1,
						{ "hold", "holdTime", "hold_time",
						  "glowTime", "glow_time" }, action.duration);
					if (not ReadColor(state, -1, "color", action.flashColor)) {
						ReadColor(state, -1, "glowColor", action.flashColor);
					}
					action.scale = ReadNumber(state, -1, "scale", 0.5);
					action.strength = ReadNumber(state, -1, "strength", 2.5);
					action.frequency = ReadNumber(state, -1, "frequency", 30.0);
					action.width = ReadNumber(state, -1, "width", 16.0);
					action.roundTrips = static_cast<int>(ReadNumber(state, -1, "roundTrips", 3));
					action.durationPerTrip = ReadNumber(state, -1, "durationPerTrip", ReadNumber(state, -1, "duration", 0.16));
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
			[](const Action& left, const Action& right) { return left.executeTime < right.executeTime; });
		_endTime = _duration;
		for (const auto& action : _actions) {
			if (action.isFinally) _endTime = Max(_endTime, action.executeTime);
		}
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
			if (auto target = ResolveTarget(action.target)) {
				target->PlayAnimation(action.animation, action.loop, action.reverse);
				if (action.hasAfterPosition || action.hasAfterOffset) {
					RegisterAnimationFinishedAction(target, action.animation, action);
				}
			}
		}
		else if (action.type == "position") {
			if (auto target = ResolveTarget(action.target)) {
				const auto& initialPosition = target == _actor
					? _actorInitialAnimationPosition : _targetInitialAnimationPosition;
				if (action.resetPosition) {
					*target->GetAnimationPositionPointer() = initialPosition;
				}
				else if (action.hasPosition) {
					*target->GetAnimationPositionPointer() = action.position;
				}
			}
		}
		else if (action.type == "position_tween") {
			if (auto target = ResolveTarget(action.target)) {
				const auto& initialPosition = target == _actor
					? _actorInitialAnimationPosition : _targetInitialAnimationPosition;
				if (action.resetPosition) {
					*target->GetAnimationPositionPointer() = initialPosition;
				}
				if (action.hasPosition) {
					StartPositionTween(target, initialPosition, action);
				}
			}
		}
		else if (action.type == "effect" && not action.effect.empty()) {
			auto effect = CreateScene<EffekseerEmitter2D>();
			effect->SetEffect(action.effect);
			if (_actor) effect->SetGlobalPosition(_actor->GetGlobalPosition() + action.offset);
			effect->Play();
			_effects.push_back(effect);
			if (action.trigger >= 0) {
				_effectTriggers.push_back({
					effect,
					action.executeTime + Max(action.triggerTime, 0.0),
					action.trigger,
					false
					});
			}
			for (const auto& [relativeTime, trigger] : action.triggers) {
				_effectTriggers.push_back({
					effect,
					action.executeTime + Max(relativeTime, 0.0),
					trigger,
					false
					});
			}
		}
		else if (action.type == "pickup" && not action.pickupTextures.empty()) {
			const int textureIndex = GetRandomInt(0, static_cast<int>(action.pickupTextures.size()));
			auto pickup = CreateScene<Pickup>();
			pickup->SetPicture(action.pickupTextures[textureIndex]);
			Vector2D displayPosition = action.hasPosition
				? action.position : Vector2D();
			if (not action.hasPosition) {
				if (auto target = ResolveTarget(action.target)) {
					displayPosition = target->GetGlobalPosition();
				}
			}
			pickup->SetGlobalPosition(displayPosition + action.offset);
			_pickups.push_back(pickup);
			CreateTimer(Max(action.duration, 0.0), [this, pickup]() {
				pickup->Free();
				std::erase(_pickups, pickup);
				});
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
		else if (action.type == "rustle") {
			if (auto target = ResolveTarget(action.target)) {
				auto tweenGroup = CreateTweenGroup();
				const double w = action.width;
				const int trips = action.roundTrips;
				const double durationPerTrip = 0.0 < action.durationPerTrip ? action.durationPerTrip : 0.16;
				if (not (trips < 1) && 0.0 < w && 0.0 < durationPerTrip) {
					const double quarterTrip = durationPerTrip * 0.25;
					const double halfTrip = durationPerTrip * 0.5;
					for (int i = 0; i < trips; ++i) {
						tweenGroup->CreateTween(&target->offset, Vector2D(w, 0.0), quarterTrip, Easing::Linear);
						tweenGroup->CreateTween(&target->offset, Vector2D(-w, 0.0), halfTrip, Easing::Linear);
						tweenGroup->CreateTween(&target->offset, Vector2D(0.0, 0.0), quarterTrip, Easing::Linear);
					}
					tweenGroup->CreateTween(&target->offset, Vector2D(0.0, 0.0), 0.001, Easing::Linear);
				}
				else {
					target->offset = Vector2D(0.0, 0.0);
				}
			}
		}
		else if (action.type == "screen_flash"
			|| action.type == "flash"
			|| action.type == "white_flash") {
			StartFlash(action.flashColor, action.fadeIn,
				action.holdTime, action.fadeOut, action.flashUnderEffects);
		}
	}

	void BattleAttack::RegisterAnimationFinishedAction(
		const SharedPtr<BattleCharacter>& target, int animation, const Action& action) {
		auto handled = std::make_shared<bool>(false);
		const int id = target->AnimationFinished.Connect(
			[this, target, animation, action, handled](int finishedAnimation) {
				if (*handled || finishedAnimation != animation) return;
				*handled = true;
				ApplyAfterAnimationPosition(target, action);
			});
		_animationFinishedConnections.push_back({ target, id });
	}

	void BattleAttack::StopAnimationFinishedConnections() {
		for (const auto& connection : _animationFinishedConnections) {
			if (connection.target && connection.id != -1) {
				connection.target->AnimationFinished.Disconnect(connection.id);
			}
		}
		_animationFinishedConnections.clear();
	}

	void BattleAttack::ApplyAfterAnimationPosition(
		const SharedPtr<BattleCharacter>& target, const Action& action) {
		auto position = target->GetAnimationPositionPointer();
		Vector2D destination = action.hasAfterPosition ? action.afterPosition : *position;
		if (action.hasAfterOffset) destination += action.afterOffset;
		if (action.afterDuration <= 0.0) {
			*position = destination;
			return;
		}
		CreateTweenGroup()->CreateTween(position, destination, action.afterDuration,
			Easing::GetEasingFunction(action.afterEasing));
	}

	void BattleAttack::StartPositionTween(const SharedPtr<BattleCharacter>& target,
		const Vector2D& initialPosition, const Action& action) {
		auto tweenGroup = CreateTweenGroup();
		auto position = target->GetAnimationPositionPointer();
		tweenGroup->CreateTween(position, action.position, action.duration,
			Easing::GetEasingFunction(action.easing));
		if (action.restorePosition) {
			tweenGroup->CreateTween(position, initialPosition, action.duration,
				Easing::GetEasingFunction(action.easing));
		}
	}

	void BattleAttack::StopPickups() {
		for (const auto& pickup : _pickups) {
			if (pickup) pickup->Free();
		}
		_pickups.clear();
	}

	void BattleAttack::StopTimelineEffects() {
		StopAnimationFinishedConnections();
		StopEffects();
		StopPickups();
		StopBlur();
		StopConcentrationLines();
		StopShake();
		StopTimeScale();
		StopFlash();
	}

	void BattleAttack::_Update(float dt) {
		if (_finished) return;
		_elapsed += dt;
		UpdateEffectTriggers();
		UpdateFlash(dt);
		UpdateBlur(dt);
		UpdateConcentrationLines(dt);
		UpdateShake(dt);
		for (auto& action : _actions) {
			if (action.isFinally) continue;
			if (action.executed || _elapsed < action.executeTime) continue;
			action.executed = true;
			Execute(action);
		}
		if (not _mainFinished && not (_elapsed < _duration)) {
			_mainFinished = true;
			StopTimelineEffects();
		}
		for (auto& action : _actions) {
			if (not action.isFinally) continue;
			if (action.executed || _elapsed < action.executeTime) continue;
			action.executed = true;
			Execute(action);
		}
		if (not (_elapsed < _endTime)) {
			_finished = true;
			StopTimelineEffects();
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

	void BattleAttack::StartFlash(const float color[4],
		double fadeIn, double holdTime, double fadeOut, bool underEffects) {
		StopFlash();
		_flashActive = true;
		_flashElapsed = 0.0;
		_flashFadeIn = Max(fadeIn, 0.0);
		_flashHoldTime = Max(holdTime, 0.0);
		_flashFadeOut = Max(fadeOut, 0.0);
		_flashOpacity = color[3];
		const auto windowSize = PropertyManager.GetSettingWindowSize();
		_flashRect = CreateScene<ColorRect>();
		_flashRect->SetSize(windowSize)
			->SetColor(Vector3D(color[0] * 255.0f, color[1] * 255.0f,
				color[2] * 255.0f))
			->SetPosition({ 0.0, 0.0 })
			->SetUIMode(not underEffects)
			->IgnoreCamera();
		UpdateFlash(0.0f);
	}

	void BattleAttack::UpdateFlash(float dt) {
		if (not _flashActive or not _flashRect) return;
		_flashElapsed += dt;

		double progress = 1.0;
		if (_flashElapsed < _flashFadeIn) {
			progress = _flashFadeIn <= 0.0
				? 1.0
				: Clamp(_flashElapsed / _flashFadeIn, 0.0, 1.0);
		}
		else {
			const double fadeOutElapsed =
				_flashElapsed - _flashFadeIn - _flashHoldTime;
			if (fadeOutElapsed > 0.0) {
				progress = _flashFadeOut <= 0.0
					? 0.0
					: Clamp(1.0 - fadeOutElapsed / _flashFadeOut, 0.0, 1.0);
			}
		}

		const int alpha = static_cast<int>(Clamp(
			progress * _flashOpacity * 255.0, 0.0, 255.0));
		_flashRect->SetAlpha(alpha);

		const double totalTime =
			_flashFadeIn + _flashHoldTime + _flashFadeOut;
		if (_flashElapsed >= totalTime) {
			StopFlash();
		}
	}

	void BattleAttack::StopFlash() {
		if (_flashRect) _flashRect->Free();
		_flashActive = false;
		_flashElapsed = 0.0;
		_flashFadeIn = 0.0;
		_flashHoldTime = 0.0;
		_flashFadeOut = 0.0;
		_flashOpacity = 1.0f;
		_flashRect.reset();
	}

	void BattleAttack::StartTimeScale(double duration, double scale) {
		StopTimeScale();
		auto controller = CreateScene<TimeScaleController>();
		controller->SetIgnoreTimeScale(true);
		controller->Start(duration, scale);
		_timeScaleController = controller;
	}

	void BattleAttack::StopTimeScale() {
		if (_timeScaleController) {
			_timeScaleController->Stop();
		}
		_timeScaleController.reset();
	}
}
