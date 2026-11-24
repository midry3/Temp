#include "EnemyBattleCharacter.h"
#include "../../BattleAttack.h"
#include <Animation/Tween.h>

namespace Game {
	void EnemyBattleCharacter::_Load() {
		BattleCharacter::_Load();

		Damaged += [this]() {
			PlayRustleAnimation(_rustleWidth, _rustleRoundTrips, _rustleDurationPerTrip);
			};

		Died += [this]() {
			PlayRustleAnimation(_rustleWidth, 3.0, _rustleDurationPerTrip*4);
			CreateTweenGroup()->CreateTween(&alpha, 0, 3.0, Easing::EaseInCubic);
			};
	}

	void EnemyBattleCharacter::AddSkill(const SkillInfo& skill) {
		auto skills = GetSkills();
		skills.push_back(skill);
		SetSkills(skills);
	}

	std::optional<SkillInfo> EnemyBattleCharacter::SelectSkill(const SharedPtr<BattleCharacter>& target) {
		if (_skillSelector) {
			return _skillSelector(*this, target);
		}
		const auto& skills = GetSkills();
		if (skills.empty()) {
			return std::nullopt;
		}
		return skills.front();
	}

	std::vector<BoardOffset> EnemyBattleCharacter::GetRelativeMoveOffsets() const {
		return {
			{ -1, -1 }, { 0, -1 }, { 1, -1 },
			{ -1,  0 },            { 1,  0 },
			{ -1,  1 }, { 0,  1 }, { 1,  1 }
		};
	}

	SharedPtr<TweenGroup> EnemyBattleCharacter::PlayRustleAnimation(double width, int roundTrips, double durationPerTrip, const std::function<void()>& onComplete) {
		auto tweenGroup = CreateTweenGroup();
		if (roundTrips < 1 || not (0.0 < width) || not (0.0 < durationPerTrip)) {
			offset = Vector2D(0.0, 0.0);
			if (onComplete) onComplete();
			return tweenGroup;
		}

		const double quarterTrip = durationPerTrip * 0.25;
		const double halfTrip = durationPerTrip * 0.5;

		for (int i = 0; i < roundTrips; ++i) {
			// 0 -> +width
			tweenGroup->CreateTween(&offset, Vector2D(width, 0.0), quarterTrip, Easing::Linear);
			// +width -> -width
			tweenGroup->CreateTween(&offset, Vector2D(-width, 0.0), halfTrip, Easing::Linear);
			// -width -> 0
			tweenGroup->CreateTween(&offset, Vector2D(0.0, 0.0), quarterTrip, Easing::Linear);
		}

		// 最後に確実に offset = {0, 0} へ戻す
		tweenGroup->CreateTween(&offset, Vector2D(0.0, 0.0), 0.001, Easing::Linear);

		if (onComplete) {
			tweenGroup->OnComplete(onComplete);
		}
		return tweenGroup;
	}

	void EnemyBattleCharacter::ExecuteTurn(const SharedPtr<BattleCharacter>& target, const std::function<void()>& onComplete) {
		const auto skillOpt = SelectSkill(target);
		if (not skillOpt.has_value()) {
			PlayRustleAnimation(_rustleWidth, _rustleRoundTrips, _rustleDurationPerTrip, [this, target, onComplete]() {
				if (target) {
					target->ApplyDamage(GetState().atk);
				}
				TurnFinished.Emit();
				if (onComplete) onComplete();
			});
			return;
		}

		const SkillInfo skill = *skillOpt;
		SkillExecuted.Emit(skill, target);

		if (not skill.directionFile.empty()) {
			if (not _attackScene) {
				_attackScene = CreateScene<BattleAttack>();
			}
			_attackScene->Completed().Connect([this, onComplete]() {
				_attackScene->Disable();
				TurnFinished.Emit();
				if (onComplete) onComplete();
			}, true);
			SharedPtr<BattleCharacter> selfChar = nullptr;
			if (not weak_from_this().expired()) {
				selfChar = std::dynamic_pointer_cast<BattleCharacter>(shared_from_this());
			}
			_attackScene->Start(selfChar, { target }, skill);
		}
		else {
			PlayRustleAnimation(_rustleWidth, _rustleRoundTrips, _rustleDurationPerTrip, [this, target, onComplete]() {
				if (target) {
					target->ApplyDamage(GetState().atk);
				}
				TurnFinished.Emit();
				if (onComplete) onComplete();
			});
		}
	}
}
