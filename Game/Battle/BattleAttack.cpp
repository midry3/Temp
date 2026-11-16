#include "BattleAttack.h"
#include <Effekseer/EffekseerEmitter.h>
#include <lua.hpp>
#include <algorithm>
#include <iostream>

namespace Game {
	namespace {
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
		if (luaL_dofile(state, file.c_str()) != LUA_OK) {
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
					action.x = static_cast<float>(ReadNumber(state, -1, "x"));
					action.y = static_cast<float>(ReadNumber(state, -1, "y"));
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
			effect->SetPosition({ action.x, action.y });
			effect->Play();
			_effects.push_back(effect);
		}
	}

	void BattleAttack::_Update(float dt) {
		if (_finished) return;
		_elapsed += dt;
		for (auto& action : _actions) {
			if (not action.executed && _elapsed >= action.time) {
				action.executed = true;
				Execute(action);
			}
		}
		if (_elapsed >= _duration) {
			_finished = true;
			StopEffects();
			_completed.Emit();
		}
	}
}
