#include "BattleBase.h"
#include <Scene/Container/Animation/AnimatedGrid.h>
#include <Game/Data/CommonData.h>
#include "WinScreen.h"

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
				_skillSelect->SetAvailableMP(_allies[_currentCharacterIndex]->GetState().currentMP);
				_skillSelect->SetSkills(_allies[_currentCharacterIndex]->GetSkills());
			}
			CreateTimer(0.5, [this]() {
				_skillSelect->In();
			});
			};

		_usingSkillView = CreateScene<AnimatedUI>();
		_usingSkillView->SetInTime(0.5)->SetOutTime(0.1);

		_allySkillPanel = _usingSkillView->CreateScene<UsingSkillPanel>();
		_allySkillPanel->SetPosition({ 450, 20 });

		_usingSkillView->Out();

		_state[Opening].Entered += [this]() {
			CreateTimer(1.0, [this]() {
				_state >> Command;
			});
			};
		_state[Command].Entered += [this]() {
			_currentCharacterIndex = 0;
			_selectedSkills.clear();
			_usingSkillView->Out();
			_battleUI->In();
			_hud->UpdateView();
			for (const auto& ally : _allies) {
				ally->PlayAnimation(0, true);
			}
			for (const auto& enemy : _enemies) {
				enemy->PlayAnimation(0, true);
			};
			};
		_state[Won].Entered += [this]() {
			CommonData::LastBattleEnemyDefeated = true;
			_usingSkillView->Out();
			_battleUI->Out();
			CreateScene<WinScreen>();
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
				if (IsDefeatedAllEnemies()) {
					_state >> Won;
				}
				else if (NextCharaExists()) {
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
		_allySkillPanel->SetSkill(_selectedSkills[_currentCharacterIndex]);
		_usingSkillView->In();
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
			ally->StatChanged.Connect([this]() {
				_hud->UpdateView();
				});
			alliesGrid->AddScene(ally);
		}
		for (const auto& enemy : _enemies) {
			enemy->StatChanged.Connect([this]() {
				_hud->UpdateView();
				});
			enemiesGrid->AddScene(enemy);
		}

		_hud->SetAllies(_allies);
		_hud->SetEnemies(_enemies);
	}

	bool BattleBase::NextCharaExists() {
		return _currentCharacterIndex + 1 < static_cast<int>(_allies.size());
	}

	bool BattleBase::IsDefeatedAllEnemies() {
		for (const auto& enemy : _enemies) {
			if (not enemy->IsDead()) {
				return false;
			}
		}
		return true;
	}
}
