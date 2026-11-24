#include "BattleBase.h"
#include <Scene/Container/Animation/AnimatedGrid.h>
#include <Game/Data/CommonData.h>
#include "WinScreen.h"
#include <Scene/Sound/SoundPlayer.h>
#include <Math/Utils.h>
#include <algorithm>

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
			const auto candidates = GetTargetCandidates(_allies[_currentCharacterIndex], skill);
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
			// 並びを選択順に戻す（選択先頭の味方が最前列）
			_allyFormation.assign(_allies.rbegin(), _allies.rend());
			ApplyAllyFormation(true);
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

		_state[Lost].Entered += [this]() {
			_usingSkillView->Out();
			_battleUI->Out();
			};

		_state >> Opening;

		if (not _allies.empty() || not _enemies.empty()) {
			PutCharacters();
		}
	}

	void BattleBase::BuildActionQueue() {
		_actionQueue.clear();
		_actionIndex = 0;

		// 味方分は選択済みのスキルと対象を使う
		for (size_t i = 0; i < _allies.size(); ++i) {
			if (not _allies[i] || _selectedSkills.size() <= i) continue;
			QueuedAction queued;
			queued.actor = _allies[i];
			queued.skill = _selectedSkills[i];
			if (i < _selectedTargets.size()) queued.targets = _selectedTargets[i];
			if (queued.targets.empty() && not _enemies.empty()) {
				queued.targets.push_back(_enemies.front());
			}
			_actionQueue.push_back(queued);
		}

		// 敵分はAIがスキルと対象を選ぶ
		for (const auto& enemy : _enemies) {
			if (not enemy || enemy->IsDead()) continue;
			const auto skill = ChooseEnemySkill(enemy);
			if (not skill.has_value()) continue;
			QueuedAction queued;
			queued.actor = enemy;
			queued.skill = skill.value();
			queued.targets = ChooseEnemyTargets(enemy, queued.skill);
			if (not queued.targets.empty()) {
				_actionQueue.push_back(queued);
			}
		}

		// 行動順は speed の大きい順。同速の場合は味方を優先する
		std::stable_sort(_actionQueue.begin(), _actionQueue.end(),
			[this](const QueuedAction& a, const QueuedAction& b) {
				const int speedA = a.actor ? a.actor->GetState().speed : 0;
				const int speedB = b.actor ? b.actor->GetState().speed : 0;
				if (speedA != speedB) return speedB < speedA;
				const bool aIsEnemy = IsEnemyCharacter(a.actor);
				const bool bIsEnemy = IsEnemyCharacter(b.actor);
				if (aIsEnemy != bIsEnemy) return bIsEnemy;
				return false;
			});

		// 味方の並びを行動順にする（最初に行動する味方が最前列）
		std::vector<SharedPtr<BattleCharacter>> allyOrder;
		for (const auto& queued : _actionQueue) {
			if (queued.actor && not IsEnemyCharacter(queued.actor)) {
				allyOrder.push_back(queued.actor);
			}
		}
		for (const auto& ally : _allies) {
			if (std::find(allyOrder.begin(), allyOrder.end(), ally) == allyOrder.end()) {
				allyOrder.push_back(ally);
			}
		}
		_allyFormation.assign(allyOrder.rbegin(), allyOrder.rend());
		ApplyAllyFormation(true);
	}

	void BattleBase::SetAllyFront(const SharedPtr<BattleCharacter>& character) {
		const auto it = std::find(_allyFormation.begin(), _allyFormation.end(), character);
		if (it == _allyFormation.end()) return;
		std::rotate(_allyFormation.begin(), it + 1, _allyFormation.end());
	}

	void BattleBase::ApplyAllyFormation(bool animate) {
		for (size_t i = 0; i < _allyFormation.size(); ++i) {
			const auto& ally = _allyFormation[i];
			if (not ally) continue;
			const Vector2D slot(static_cast<double>(i) * ALLY_SPACING, 0.0);
			if (animate) {
				auto tweenGroup = ally->CreateTweenGroup();
				tweenGroup->CreateTween(&ally->position, slot, FORMATION_ANIMATION_TIME, Easing::EaseInOutQuad);
			}
			else {
				ally->position = slot;
			}
		}
		ApplyAllyDimming();
	}

	void BattleBase::ApplyAllyDimming() {
		for (size_t i = 0; i < _allyFormation.size(); ++i) {
			const auto& ally = _allyFormation[i];
			if (not ally) continue;
			const bool isFront = i + 1 == _allyFormation.size();
			ally->SetModulateAll(isFront
				? Vector3D(255, 255, 255)
				: Vector3D(FORMATION_DIM_VALUE, FORMATION_DIM_VALUE, FORMATION_DIM_VALUE));
			ally->OnFormationFront(isFront);
		}
	}

	void BattleBase::AdvanceAllyFormationAfterAction() {
		// 次に行動する味方を最前列にする（最前列の味方が倒れた場合も含む）
		for (size_t i = _actionIndex; i < _actionQueue.size(); ++i) {
			const auto& actor = _actionQueue[i].actor;
			if (not actor || IsEnemyCharacter(actor)) continue;
			if (not _allyFormation.empty() && _allyFormation.back() == actor) {
				ApplyAllyDimming();
			}
			else {
				SetAllyFront(actor);
				ApplyAllyFormation(true);
			}
			return;
		}
		ApplyAllyDimming();
	}

	void BattleBase::ExecuteNextAction() {
		if (IsDefeatedAllEnemies()) {
			_state >> Won;
			return;
		}
		if (IsDefeatedAllAllies()) {
			_state >> Lost;
			return;
		}
		if (not _attack) {
			_attack = CreateScene<BattleAttack>();
			_attack->Completed().Connect([this]() {
				_hud->UpdateView();
				AdvanceAllyFormationAfterAction();
				ExecuteNextAction();
				});
		}

		// 行動者が死亡している、または対象が全滅している行動は飛ばす
		while (_actionIndex < _actionQueue.size()) {
			auto& queued = _actionQueue[_actionIndex];
			std::vector<SharedPtr<BattleCharacter>> aliveTargets;
			for (const auto& target : queued.targets) {
				if (target && not target->IsDead()) {
					aliveTargets.push_back(target);
				}
			}
			if (queued.actor && not queued.actor->IsDead() && not aliveTargets.empty()) {
				queued.targets = aliveTargets;
				break;
			}
			_actionIndex++;
		}
		if (_actionQueue.size() <= _actionIndex) {
			_attack->Disable();
			_state >> Command;
			return;
		}

		const auto queued = _actionQueue[_actionIndex++];
		_skillSelect->Disable();
		_state >> Direction;
		_allySkillPanel->SetSkill(queued.skill);
		_usingSkillView->In();
		_attack->Start(queued.actor, queued.targets, queued.skill);
	}

	std::optional<SkillInfo> BattleBase::ChooseEnemySkill(const SharedPtr<BattleCharacter>& enemy) const {
		// 敵はMPを無視して全スキルから選ぶ
		const auto& skills = enemy->GetSkills();
		if (skills.empty()) return std::nullopt;
		const int index = GetRandomInt(0, static_cast<int>(skills.size()));
		return skills[index];
	}

	std::vector<SharedPtr<BattleCharacter>> BattleBase::ChooseEnemyTargets(const SharedPtr<BattleCharacter>& enemy, const SkillInfo& skill) const {
		const auto candidates = GetTargetCandidates(enemy, skill);
		if (skill.targetScope == SkillTargetScope::All) return candidates;
		if (candidates.empty()) return {};
		const int index = GetRandomInt(0, static_cast<int>(candidates.size()));
		return { candidates[index] };
	}

	bool BattleBase::IsEnemyCharacter(const SharedPtr<BattleCharacter>& character) const {
		for (const auto& enemy : _enemies) {
			if (enemy == character) return true;
		}
		return false;
	}

	std::vector<SharedPtr<BattleCharacter>> BattleBase::GetTargetCandidates(const SharedPtr<BattleCharacter>& actor, const SkillInfo& skill) const {
		// targetSide は行動者から見た陣営（Enemy = 相手側、Ally = 自分側）
		const bool actorIsEnemy = IsEnemyCharacter(actor);
		const auto& opponentSide = actorIsEnemy ? _allies : _enemies;
		const auto& ownSide = actorIsEnemy ? _enemies : _allies;
		const auto& source = skill.targetSide == SkillTargetSide::Enemy ? opponentSide : ownSide;
		std::vector<SharedPtr<BattleCharacter>> result;
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
			// 選択が終わったキャラは後ろへ、次のキャラが最前列へ
			SetAllyFront(_allies[_currentCharacterIndex]);
			ApplyAllyFormation(true);
			_battleUI->In();
		}
		else {
			_currentCharacterIndex = 0;
			BuildActionQueue();
			ExecuteNextAction();
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

		_alliesRow = CreateScene<Scene>();
		_alliesRow->SetPosition({ ALLIES_X, CHARACTER_Y });
		const auto& enemiesGrid = CreateScene<AnimatedGrid>();
		enemiesGrid->SetSize({ 200, CHARACTER_GRID_HEIGHT })->SetPosition({ ENEMIES_X, CHARACTER_Y });

		for (const auto& ally : _allies) {
			ally->StatChanged.Connect([this]() {
				_hud->UpdateView();
				});
			ally->Load();
			_alliesRow->AddScene(ally);
		}

		// 初期配置は選択順（選択先頭の味方が最前列）
		_allyFormation.assign(_allies.rbegin(), _allies.rend());
		ApplyAllyFormation(false);
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

	bool BattleBase::IsDefeatedAllAllies() {
		if (_allies.empty()) return false;
		for (const auto& ally : _allies) {
			if (ally && not ally->IsDead()) {
				return false;
			}
		}
		return true;
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
