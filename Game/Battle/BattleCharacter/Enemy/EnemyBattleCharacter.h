#pragma once
#include "../BattleCharacter.h"
#include "../../../Board/BoardTypes.h"
#include <functional>
#include <optional>

class TweenGroup;

namespace Game {
	class BattleAttack;

	class EnemyBattleCharacter : public BattleCharacter {
	public:
		using SkillSelector = std::function<std::optional<SkillInfo>(const EnemyBattleCharacter&, const SharedPtr<BattleCharacter>&)>;

	private:
		double _rustleWidth = 16.0;
		int _rustleRoundTrips = 3;
		double _rustleDurationPerTrip = 0.16;
		SkillSelector _skillSelector;
		SharedPtr<BattleAttack> _attackScene;

	protected:
		void _Load() override;

	public:
		Signal<SkillInfo, SharedPtr<BattleCharacter>> SkillExecuted;
		Signal<> TurnFinished;

		void SetSkills(const std::vector<SkillInfo>& skills) { BattleCharacter::SetSkills(skills); }
		void AddSkill(const SkillInfo& skill);
		void SetSkillSelector(const SkillSelector& selector) { _skillSelector = selector; }

		void SetRustleParams(double width, int roundTrips, double durationPerTrip = 0.16) {
			_rustleWidth = width;
			_rustleRoundTrips = roundTrips;
			_rustleDurationPerTrip = durationPerTrip;
		}
		[[nodiscard]] double GetRustleWidth() const { return _rustleWidth; }
		[[nodiscard]] int GetRustleRoundTrips() const { return _rustleRoundTrips; }
		[[nodiscard]] double GetRustleDurationPerTrip() const { return _rustleDurationPerTrip; }

		[[nodiscard]] virtual std::optional<SkillInfo> SelectSkill(const SharedPtr<BattleCharacter>& target = nullptr);
		[[nodiscard]] std::optional<SkillInfo> ChooseSkill(const SharedPtr<BattleCharacter>& target = nullptr) { return SelectSkill(target); }

		[[nodiscard]] virtual std::vector<BoardOffset> GetRelativeMoveOffsets() const;

		virtual void ExecuteTurn(const SharedPtr<BattleCharacter>& target, const std::function<void()>& onComplete = nullptr);
		void TakeTurn(const SharedPtr<BattleCharacter>& target, const std::function<void()>& onComplete = nullptr) { ExecuteTurn(target, onComplete); }

		SharedPtr<TweenGroup> PlayRustleAnimation(double width, int roundTrips, double durationPerTrip = 0.16, const std::function<void()>& onComplete = nullptr);
	};
}
