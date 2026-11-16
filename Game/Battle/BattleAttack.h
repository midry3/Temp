#pragma once
#include <LogicScene.h>
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
			float x = 0.0f;
			float y = 0.0f;
			bool executed = false;
		};
		SharedPtr<BattleCharacter> _actor;
		SharedPtr<BattleCharacter> _target;
		std::vector<Action> _actions;
		std::vector<SharedPtr<EffekseerEmitter2D>> _effects;
		double _elapsed = 0.0;
		double _duration = 0.0;
		bool _finished = false;
		Signal<> _completed;

		bool LoadTimeline(const std::string& file);
		void Execute(Action& action);
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
