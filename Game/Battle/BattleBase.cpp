#include "BattleBase.h"
#include <Scene/Container/Animation/AnimatedGrid.h>
#include <Game/Data/CommonData.h>
#include "WinScreen.h"
#include "LoseScreen.h"
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
				_selectedAllyIndices.push_back(_currentCharacterIndex);
				_selectedPasses.push_back(false);
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

		_skillSelect->BackPressed += [this]() {
			// スキル選択からコマンド選択に戻る
			_skillSelect->Out();
			_battleUI->In();
			};

		_battleUI->PassPressed += [this]() {
			// 「何もしない」: スキルを選ばず、このキャラは行動しない
			_selectedSkills.push_back(SkillInfo{});
			_selectedTargets.emplace_back();
			_selectedAllyIndices.push_back(_currentCharacterIndex);
			_selectedPasses.push_back(true);
			AdvanceSelection();
			};

		_battleUI->SurrenderPressed += [this]() {
			// 「降参」: 敗北として扱う
			_targetSelect->Hide();
			_state >> Lost;
			};

		_battleUI->BackPressed += [this]() {
			GoBackSelection();
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
			_selectedSkills.clear();
			_selectedTargets.clear();
			_selectedAllyIndices.clear();
			_selectedPasses.clear();
			UpdateBackButton();
			// 死亡した味方のコマンドは飛ばす（最初の生存味方から選択を始める）
			_currentCharacterIndex = 0;
			while (_currentCharacterIndex < static_cast<int>(_allies.size())
				&& (not _allies[_currentCharacterIndex] || _allies[_currentCharacterIndex]->IsDead())) {
				_currentCharacterIndex++;
			}
			if (static_cast<int>(_allies.size()) <= _currentCharacterIndex) {
				_state >> Lost;
				return;
			}
			// 並びを選択順に戻す（選択先頭の味方が最前列。死亡した味方は並びから外す）
			_formationFrontX = ALLIES_X;
			_allyFormation.clear();
			for (auto it = _allies.rbegin(); it != _allies.rend(); ++it) {
				if (*it && not (*it)->IsDead()) _allyFormation.push_back(*it);
			}
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
			CommonData::LastBattleAlliesDefeated = true;
			_usingSkillView->Out();
			_battleUI->Out();
			CreateScene<LoseScreen>();
			};

		_state >> Opening;

		if (not _allies.empty() || not _enemies.empty()) {
			PutCharacters();
		}
	}

	void BattleBase::BuildActionQueue() {
		_actionQueue.clear();
		_actionIndex = 0;

		// 味方分は選択済みのスキルと対象を使う（選択した味方のインデックスと対応付ける）
		for (size_t k = 0; k < _selectedSkills.size(); ++k) {
			if (_selectedAllyIndices.size() <= k) continue;
			// 「何もしない」を選んだキャラは行動しない
			if (k < _selectedPasses.size() && _selectedPasses[k]) continue;
			const int allyIndex = _selectedAllyIndices[k];
			if (allyIndex < 0 || static_cast<int>(_allies.size()) <= allyIndex || not _allies[allyIndex]) continue;
			QueuedAction queued;
			queued.actor = _allies[allyIndex];
			queued.skill = _selectedSkills[k];
			if (k < _selectedTargets.size()) queued.targets = _selectedTargets[k];
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
		// 最前列の位置（行全体の原点）を必要に応じてずらす
		if (_alliesRow) {
			if (animate) {
				auto rowTween = _alliesRow->CreateTweenGroup();
				rowTween->CreateTween(&_alliesRow->position.x, _formationFrontX, FORMATION_ANIMATION_TIME, Easing::EaseInOutQuad);
			}
			else {
				_alliesRow->position.x = _formationFrontX;
			}
		}
		for (size_t i = 0; i < _allyFormation.size(); ++i) {
			const auto& ally = _allyFormation[i];
			if (not ally) continue;
			// 最前列（最後尾のキャラ）が ALLIES_X に来るよう、左方向へ並べる
			const double backCount = static_cast<double>(_allyFormation.size() - 1 - i);
			const Vector2D slot(-backCount * ALLY_SPACING, 0.0);
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
		// 敵の攻撃時（列が ALLIES_X_2 に寄っている間）は薄黒を解除する
		const bool dimEnabled = _formationFrontX == ALLIES_X;
		for (size_t i = 0; i < _allyFormation.size(); ++i) {
			const auto& ally = _allyFormation[i];
			if (not ally) continue;
			const bool isFront = i + 1 == _allyFormation.size();
			const bool bright = isFront || not dimEnabled;
			ally->SetModulateAll(bright
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
				// 位置の適用は ExecuteNextAction 側で行う（最前列Xの切り替えとまとめるため）
				SetAllyFront(actor);
				ApplyAllyDimming();
			}
			return;
		}
		ApplyAllyDimming();
	}

	void BattleBase::ApplyAttackDimming(const SharedPtr<BattleCharacter>& actor, const std::vector<SharedPtr<BattleCharacter>>& targets) {
		// 攻撃者と被弾対象以外を薄黒にする。元の色味は控えておき、演出後に戻す
		// （レア種の赤系 modulate など、キャラクター固有の色味を消さないため）
		_attackDimRestore.clear();
		auto applyTo = [this, &actor, &targets](const SharedPtr<BattleCharacter>& character) {
			if (not character) return;
			bool active = character == actor;
			if (not active) {
				for (const auto& target : targets) {
					if (target == character) active = true;
				}
			}
			_attackDimRestore[character.get()] = character->modulate;
			if (not active) {
				const auto base = character->modulate;
				character->SetModulateAll(Vector3D(base.x * ATTACK_DIM_RATE, base.y * ATTACK_DIM_RATE, base.z * ATTACK_DIM_RATE));
			}
		};
		for (const auto& ally : _allies) applyTo(ally);
		for (const auto& enemy : _enemies) applyTo(enemy);
	}

	void BattleBase::ClearAttackDimming() {
		auto restore = [this](const SharedPtr<BattleCharacter>& character) {
			if (not character) return;
			const auto it = _attackDimRestore.find(character.get());
			if (it != _attackDimRestore.end()) {
				character->SetModulateAll(it->second);
			}
		};
		for (const auto& ally : _allies) restore(ally);
		for (const auto& enemy : _enemies) restore(enemy);
		_attackDimRestore.clear();
		// 味方はフォーメーション側の薄黒（最前列以外）を改めて適用する
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
				ClearAttackDimming();
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
		// 敵の攻撃時は味方の列を右へ寄せて見やすくする
		_formationFrontX = IsEnemyCharacter(queued.actor) ? ALLIES_X_2 : ALLIES_X;
		ApplyAllyFormation(true);
		// 攻撃の場面では攻撃者と被弾対象以外を薄黒にする
		ApplyAttackDimming(queued.actor, queued.targets);
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
		// Self の対象は行動者自身のみ（候補が1体になるため、対象選択は自動的にスキップされる）
		if (skill.targetSide == SkillTargetSide::Self) {
			if (actor && not actor->IsDead()) return { actor };
			return {};
		}
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
			// 死亡した味方は飛ばして次の生存味方へ
			do {
				_currentCharacterIndex++;
			} while (not _allies[_currentCharacterIndex] || _allies[_currentCharacterIndex]->IsDead());
			// 選択が終わったキャラは後ろへ、次のキャラが最前列へ
			SetAllyFront(_allies[_currentCharacterIndex]);
			ApplyAllyFormation(true);
			UpdateBackButton();
			_battleUI->In();
		}
		else {
			_currentCharacterIndex = 0;
			_battleUI->Out();
			BuildActionQueue();
			ExecuteNextAction();
		}
	}

	void BattleBase::UpdateBackButton() {
		// 「戻る」は二番目以降のキャラの選択時にだけ有効
		_battleUI->SetBackEnabled(not _selectedAllyIndices.empty());
	}

	void BattleBase::GoBackSelection() {
		if (_selectedAllyIndices.empty()) return;
		// 一つ前のキャラの選択を取り消して、そのキャラの選択に戻る
		_currentCharacterIndex = _selectedAllyIndices.back();
		_selectedSkills.pop_back();
		_selectedTargets.pop_back();
		_selectedAllyIndices.pop_back();
		if (not _selectedPasses.empty()) _selectedPasses.pop_back();
		SetAllyFront(_allies[_currentCharacterIndex]);
		ApplyAllyFormation(true);
		UpdateBackButton();
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
			_selectedAllyIndices.push_back(_currentCharacterIndex);
			_selectedPasses.push_back(false);
			AdvanceSelection();
			return;
		}
		_targetSelect->Show(panelCandidates, [this, skill](const SharedPtr<BattleCharacter>& chosen) {
			_selectedSkills.push_back(skill);
			_selectedTargets.push_back({ chosen });
			_selectedAllyIndices.push_back(_currentCharacterIndex);
			_selectedPasses.push_back(false);
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
		// 死亡した味方は数えない
		for (int i = _currentCharacterIndex + 1; i < static_cast<int>(_allies.size()); ++i) {
			if (_allies[i] && not _allies[i]->IsDead()) return true;
		}
		return false;
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
