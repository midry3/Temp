#include "BattleBase.h"
#include <Scene/Container/Animation/AnimatedGrid.h>
#include <Game/Data/CommonData.h>
#include "WinScreen.h"
#include <Scene/Sound/SoundPlayer.h>

namespace Game {
	void BattleBase::_Load() {
		Camera.SetZoom(Vec2D_1);

		const auto& bgmPlayer = CreateScene<SoundPlayer>();
		bgmPlayer->SetSound(LoadAsset<Sound>("sound/bgm/bgm1.oga"))
			->SetLoop()
			->SetVolume(0.5f);
		bgmPlayer->Play();

		_background = CreateScene<BattleBackground>();

		_hud = CreateScene<Hud>();

		_battleUI = CreateScene<BattleUI>();
		_battleUI->SetUIMode(true);

		_skillSelect = CreateScene<SkillSelect>();
		_skillSelect->SetUIMode(true)->Disable();

		_targetSelect = CreateScene<TargetSelect>();
		_targetSelect->SetUIMode(true);

		_skillSelect->SkillSelected += [this](const SkillInfo& skill) {
			const auto candidates = GetTargetCandidates(skill);
			if (skill.targetScope == SkillTargetScope::All || not (1 < static_cast<int>(candidates.size()))) {
				// 全体対象、または単体対象でも候補が1体以下なら選択をスキップする
				_selectedSkills.push_back(skill);
				_selectedTargets.push_back(candidates);
				AdvanceSelection();
			}
			else {
				ShowTargetSelect(skill, candidates);
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
			_selectedTargets.clear();
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
		std::vector<SharedPtr<BattleCharacter>> targets;
		if (_currentCharacterIndex < static_cast<int>(_selectedTargets.size())) {
			targets = _selectedTargets[_currentCharacterIndex];
		}
		if (targets.empty() && not _enemies.empty()) {
			targets.push_back(_enemies.front());
		}
		_attack->Start(_allies[_currentCharacterIndex], targets, _selectedSkills[_currentCharacterIndex]);
	}

	std::vector<SharedPtr<BattleCharacter>> BattleBase::GetTargetCandidates(const SkillInfo& skill) const {
		std::vector<SharedPtr<BattleCharacter>> result;
		const auto& source = skill.targetSide == SkillTargetSide::Ally ? _allies : _enemies;
		for (const auto& character : source) {
			if (character && not character->IsDead()) {
				result.push_back(character);
			}
		}
		return result;
	}

	void BattleBase::AdvanceSelection() {
		if (NextCharaExists()) {
			_currentCharacterIndex++;
			_battleUI->In();
		}
		else {
			_currentCharacterIndex = 0;
			StartAttack();
		}
	}

	void BattleBase::ShowTargetSelect(const SkillInfo& skill, const std::vector<SharedPtr<BattleCharacter>>& candidates) {
		std::vector<std::pair<SharedPtr<BattleCharacter>, std::pair<Vector2D, Vector2D>>> panelCandidates;
		for (const auto& character : candidates) {
			if (not character) continue;
			const auto rect = _hud->GetCharacterPanelRect(character);
			if (rect.has_value()) {
				panelCandidates.emplace_back(character, rect.value());
			}
		}
		if (panelCandidates.empty()) {
			// パネル矩形が取れない場合は先頭を自動選択して進める
			_selectedSkills.push_back(skill);
			_selectedTargets.push_back(candidates.empty()
				? std::vector<SharedPtr<BattleCharacter>>{}
				: std::vector<SharedPtr<BattleCharacter>>{ candidates.front() });
			AdvanceSelection();
			return;
		}
		_targetSelect->Show(panelCandidates, [this, skill](const SharedPtr<BattleCharacter>& chosen) {
			_selectedSkills.push_back(skill);
			_selectedTargets.push_back({ chosen });
			AdvanceSelection();
			});
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
