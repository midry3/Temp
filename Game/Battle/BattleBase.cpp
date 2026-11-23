#include "BattleBase.h"
#include <Scene/Container/Animation/AnimatedGrid.h>

namespace Game {
	void BattleBase::_Load() {
		_background = CreateScene<BattleBackground>();

		_hud = CreateScene<Hud>();

		_battleUI = CreateScene<BattleUI>();
		_battleUI->SetUIMode(true);

		_skillSelect = CreateScene<SkillSelect>();
		_skillSelect->SetUIMode(true)->Disable();

		_skillSelect->SkillSelected += [this](const SkillInfo& skill) {
			_selectedSkills.push_back(skill);
			if (NextCharaExists()) {
				_currentCharacterIndex++;
				_battleUI->In();
			}
			else {
				_currentCharacterIndex = 0;
				StartAttack();
			}
			};

		_battleUI->AtkPressed += [this]() {
			_battleUI->Out();
			if (not _allies.empty()) {
				_skillSelect->SetSkills(_allies[_currentCharacterIndex]->GetSkills());
			}
			CreateTimer(0.5, [this]() {
				_skillSelect->In();
			});
			};

		_state[Opening].Entered += [this]() {
			CreateTimer(1.0, [this]() {
				_state >> Command;
			});
			};
		_state[Command].Entered += [this]() {
			_currentCharacterIndex = 0;
			_selectedSkills.clear();
			_battleUI->In();
			_hud->UpdateView();
			for (const auto& ally : _allies) {
				ally->PlayAnimation(0, true);
			}
			for (const auto& enemy : _enemies) {
				enemy->PlayAnimation(0, true);
			};
			};
		_state >> Opening;

		if (not _allies.empty() || not _enemies.empty()) {
			PutCharacters();
		}
	}

	void BattleBase::StartAttack() {
		if (_allies.empty() || _enemies.empty()) return;
		_skillSelect->Disable();
		if (not _attack) {
			_attack = CreateScene<BattleAttack>();
			_attack->Completed().Connect([this]() {
				if (NextCharaExists()) {
					_hud->UpdateView();
					_currentCharacterIndex++;
					StartAttack();
				}
				else {
					_attack->Disable();
					_state >> Command;
				}
				});
		}
		_state >> Direction;
		_attack->Start(_allies[_currentCharacterIndex], _enemies.front(), _selectedSkills[_currentCharacterIndex]);
	}

	void BattleBase::PutCharacters() {
		if (_charactersPut) return;
		_charactersPut = true;

		const auto& alliesGrid = CreateScene<AnimatedGrid>();
		alliesGrid->SetSize({ 200, CHARACTER_GRID_HEIGHT })->SetPosition({ ALLIES_X, CHARACTER_Y });
		const auto& enemiesGrid = CreateScene<AnimatedGrid>();
		enemiesGrid->SetSize({ 200, CHARACTER_GRID_HEIGHT })->SetPosition({ ENEMIES_X, CHARACTER_Y });

		for (const auto& ally : _allies) {
			alliesGrid->AddScene(ally);
		}
		for (const auto& enemy : _enemies) {
			//enemy->SetScale({ Abs(enemy->GetScale().x), enemy->GetScale().y });
			enemiesGrid->AddScene(enemy);
		}

		_hud->SetAllies(_allies);
		_hud->SetEnemies(_enemies);
	}

	bool BattleBase::NextCharaExists() {
		return _currentCharacterIndex + 1 < static_cast<int>(_allies.size());
	}
}
