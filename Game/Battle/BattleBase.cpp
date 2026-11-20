#include "BattleBase.h"
#include <Scene/Container/Animation/AnimatedGrid.h>

namespace Game {
	void BattleBase::_Load() {
		_background = CreateScene<BattleBackground>();

		_battleUI = CreateScene<BattleUI>();
		_battleUI->SetUIMode(true)
			;//->Disable();

		_skillSelect = CreateScene<SkillSelect>();
		_skillSelect->SetUIMode(true)
			->Disable();

		_skillSelect->SkillSelected += [this](const SkillInfo& skill) {
			StartAttack(skill);
			};

		_battleUI->AtkPressed += [this]() {
			_battleUI->Out();
			if (not _allies.empty()) {
				_skillSelect->SetSkills(_allies.front()->GetSkills());
			}
			_skillSelect->In();
			};

		_state[Opening].Entered += [this]() {
			CreateTimer(1.0, [this]() {
				_state >> Command;
			});
			};
		_state[Command].Entered += [this]() {
			_battleUI->In();
			for (const auto& ally : _allies) {
				ally->PlayAnimation(0, true);
			}
			for (const auto& enemy : _enemies) {
				enemy->PlayAnimation(0, true);
			};
			};
		_state >> Opening;
	}

	void BattleBase::StartAttack(const SkillInfo& skill) {
		if (_allies.empty() || _enemies.empty()) return;
		_skillSelect->Disable();
		if (not _attack) {
			_attack = CreateScene<BattleAttack>();
			_attack->Completed().Connect([this]() {
				_attack->Disable();
				_state >> Command;
				});
		}
		_state >> Direction;
		_attack->Start(_allies.front(), _enemies.front(), skill);
	}

	void BattleBase::PutCharacters() {
		const auto& alliesGrid = CreateScene<AnimatedGrid>();
		alliesGrid->SetSize({ 200, CHARACTER_GRID_HEIGHT })
			->SetPosition({ ALLIES_X, CHARACTER_Y });
		const auto& enemiesGrid = CreateScene<AnimatedGrid>();
		enemiesGrid->SetSize({ 200, CHARACTER_GRID_HEIGHT })
			->SetPosition({ ENEMIES_X, CHARACTER_Y });

		for (const auto& ally : _allies) {
			alliesGrid->AddScene(ally);
		}
		for (const auto& enemy : _enemies) {
			enemy->SetScale({ Abs(enemy->GetScale().x), enemy->GetScale().y });
			enemiesGrid->AddScene(enemy);
		}
	}
};