#include "BattleBase.h"
#include <Scene/Container/Animation/AnimatedGrid.h>

namespace Game {
	void BattleBase::_Load() {
		_battleUI = CreateScene<BattleUI>();
		_battleUI->SetUIMode(true)
			;//->Disable();

		_skillSelect = CreateScene<SkillSelect>();
		_skillSelect->SetUIMode(true)
			->Disable();

		_battleUI->AtkPressed += [this]() {
			_battleUI->Out();
			_skillSelect->In();
			};
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
}