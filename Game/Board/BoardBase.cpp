#include "BoardBase.h"
#include "BoardInfoPanel.h"
#include "BoardWinScreen.h"
#include "CoopSelectModal.h"
#include "../Battle/BattleBase.h"
#include "../Battle/BattleLoadingScene.h"
#include "../Battle/BattleStartScene.h"
#include "../Battle/BattleCharacter/BattleCharacterFactory.h"
#include "../Battle/BattleCharacter/Enemy/EnemyBattleCharacter.h"
#include <Animation/Tween.h>
#include <Asset/AssetManager.h>
#include <Scene/Texture/Sprite2D.h>
#include <Game/Data/CommonData.h>
#include <SceneManager.h>
#include <lua.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace Game {
	namespace {
		std::string ReadString(lua_State* state, int table, const char* key, const std::string& fallback = "") {
			lua_getfield(state, table, key);
			const char* value = lua_tostring(state, -1);
			const std::string result = value ? value : fallback;
			lua_pop(state, 1);
			return result;
		}

		int ReadInteger(lua_State* state, int table, const char* key, int fallback = 0) {
			lua_getfield(state, table, key);
			const int result = lua_isnumber(state, -1) ? static_cast<int>(lua_tointeger(state, -1)) : fallback;
			lua_pop(state, 1);
			return result;
		}

		double ReadNumber(lua_State* state, int table, const char* key, double fallback = 0.0) {
			lua_getfield(state, table, key);
			const double result = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : fallback;
			lua_pop(state, 1);
			return result;
		}

		bool ReadBool(lua_State* state, int table, const char* key, bool fallback = false) {
			lua_getfield(state, table, key);
			const bool result = lua_isboolean(state, -1) ? lua_toboolean(state, -1) != 0 : fallback;
			lua_pop(state, 1);
			return result;
		}

		void ReadPosition(lua_State* state, int table, double& x, double& y) {
			lua_getfield(state, table, "position");
			if (lua_istable(state, -1)) {
				x = ReadNumber(state, -1, "x", x);
				y = ReadNumber(state, -1, "y", y);
			}
			lua_pop(state, 1);
		}

		BoardTerrain ParseTerrain(const std::string& value) {
			if (value == "disabled") return BoardTerrain::Disabled;
			if (value == "other") return BoardTerrain::Other;
			return BoardTerrain::Normal;
		}

		BoardTeam ParseTeam(const std::string& value) {
			return value == "enemy" ? BoardTeam::Enemy : BoardTeam::Ally;
		}

		BoardMoveType ParseMoveType(const std::string& value) {
			if (value == "diagonal_unlimited") return BoardMoveType::DiagonalUnlimited;
			if (value == "range_two") return BoardMoveType::RangeTwo;
			if (value == "orthogonal_unlimited") return BoardMoveType::OrthogonalUnlimited;
			return BoardMoveType::Adjacent;
		}
	}

	void BoardBase::_Load() {
		CommonData::CurrentBoard = std::dynamic_pointer_cast<BoardBase>(shared_from_this());
		_infoPanel = CreateScene<BoardInfoPanel>();

		_bgmPlayer = CreateScene<SoundPlayer>();
		_bgmPlayer->SetSound(LoadAsset<Sound>("sound/bgm/bgm1.oga"))
			->SetLoop()
			->SetVolume(0.5);
		
		EnteredMainScene.Connect([this]() {
			OnReturnedFromBattle();
			_bgmPlayer->FadeAndPlay(0.5f);
			});
	}

	void BoardBase::_Update(float dt) {
		Scene::_Update(dt);
		if (_pendingTransition) {
			auto transition = std::move(_pendingTransition);
			_pendingTransition = nullptr;
			transition();
			return;
		}
	}

	bool BoardBase::LoadBoard(const std::string& file) {
		lua_State* state = luaL_newstate();
		if (not state) return false;
		luaL_openlibs(state);
		const auto resolvedFile = AssetManager.ResolvePath(file);
		if (luaL_dofile(state, resolvedFile.string().c_str()) != LUA_OK) {
			std::cerr << "Board Lua load error (" << file << "): "
				<< lua_tostring(state, -1) << std::endl;
			lua_close(state);
			return false;
		}

		lua_getglobal(state, "board");
		if (not lua_istable(state, -1)) {
			lua_pop(state, 1);
			lua_close(state);
			return false;
		}

		_width = ReadInteger(state, -1, "width");
		_height = ReadInteger(state, -1, "height");
		_tileSize.x = ReadInteger(state, -1, "tileSize", 64);
		_tileSize.y = _tileSize.x;
		double unusedX = 0.0;
		double unusedY = 0.0;
		ReadPosition(state, -1, unusedX, unusedY);
		_boardPosition = Vector2D(0.0, 0.0);
		if (not (0 < _width) || not (0 < _height)) {
			lua_pop(state, 1);
			lua_close(state);
			return false;
		}

		std::map<BoardTerrain, std::string> textures;
		lua_getfield(state, -1, "textures");
		if (lua_istable(state, -1)) {
			textures[BoardTerrain::Normal] = ReadString(state, -1, "normal");
			textures[BoardTerrain::Disabled] = ReadString(state, -1, "disabled", textures[BoardTerrain::Normal]);
			textures[BoardTerrain::Other] = ReadString(state, -1, "other", textures[BoardTerrain::Normal]);
		}
		lua_pop(state, 1);

		std::vector<BoardTileData> tiles;
		lua_getfield(state, -1, "tiles");
		if (lua_istable(state, -1)) {
			const int count = static_cast<int>(lua_rawlen(state, -1));
			for (int index = 1; index <= count; index++) {
				lua_rawgeti(state, -1, index);
				if (lua_isstring(state, -1)) {
					const auto terrain = ParseTerrain(lua_tostring(state, -1));
					const auto texture = textures.find(terrain);
					tiles.push_back({ terrain, texture == textures.end() ? "" : texture->second });
				}
				else if (lua_istable(state, -1)) {
					const auto terrainName = ReadString(state, -1, "terrain", "normal");
					auto terrain = ParseTerrain(terrainName);
					const bool passable = ReadBool(state, -1, "passable", terrain != BoardTerrain::Disabled);
					if (not passable) terrain = BoardTerrain::Disabled;
					else if (terrain == BoardTerrain::Disabled) terrain = BoardTerrain::Normal;
					const auto explicitTexture = ReadString(state, -1, "texture");
					const auto texture = textures.find(terrain);
					tiles.push_back({ terrain, explicitTexture.empty()
						? (texture == textures.end() ? "" : texture->second) : explicitTexture });
				}
				else {
					const auto texture = textures.find(BoardTerrain::Normal);
					tiles.push_back({ BoardTerrain::Normal, texture == textures.end() ? "" : texture->second });
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);

		std::vector<BoardObjectData> objects;
		lua_getfield(state, -1, "objects");
		if (lua_istable(state, -1)) {
			const int count = static_cast<int>(lua_rawlen(state, -1));
			for (int index = 1; index <= count; ++index) {
				lua_rawgeti(state, -1, index);
				if (lua_istable(state, -1)) {
					const double scaleVal = ReadNumber(state, -1, "scale", 1.0);
					objects.push_back({
						ReadString(state, -1, "texture"),
						ReadInteger(state, -1, "x"),
						ReadInteger(state, -1, "y"),
						ReadInteger(state, -1, "width", 1),
						ReadInteger(state, -1, "height", 1),
						ReadNumber(state, -1, "scaleX", scaleVal),
						ReadNumber(state, -1, "scaleY", scaleVal)
					});
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);

		std::vector<BoardUnitData> units;
		lua_getfield(state, -1, "units");
		if (lua_istable(state, -1)) {
			const int count = static_cast<int>(lua_rawlen(state, -1));
			for (int index = 1; index <= count; index++) {
				lua_rawgeti(state, -1, index);
				if (lua_istable(state, -1)) {
					BoardUnitData unit;
					unit.id = ReadString(state, -1, "id");
					unit.displayName = ReadString(state, -1, "displayName");
					unit.characterType = ParseBoardCharacterType(ReadString(state, -1, "character", "plea"));
					unit.team = ParseTeam(ReadString(state, -1, "team", "ally"));
					unit.moveType = ParseMoveType(ReadString(state, -1, "move", "adjacent"));
					// キャラ固有の移動ルール（Lua の move 指定より優先する）
					if (unit.characterType == BoardCharacterType::Reko) unit.moveType = BoardMoveType::RangeTwo;
					if (unit.characterType == BoardCharacterType::Gou) unit.moveType = BoardMoveType::OrthogonalUnlimited;
					unit.texture = ReadString(state, -1, "icon", ReadString(state, -1, "texture"));
					unit.x = ReadInteger(state, -1, "x");
					unit.y = ReadInteger(state, -1, "y");
					units.push_back(unit);
				}
				lua_pop(state, 1);
			}
		}
		lua_pop(state, 1);
		lua_pop(state, 1);
		lua_close(state);

		UpdateLayout();
		BuildTiles(textures, tiles);
		BuildObjects(objects);
		BuildUnits(units);
		UpdateEnemyIntentVisuals();
		return true;
	}

	void BoardBase::UpdateLayout() {
		const Vector2D size(static_cast<double>(_width) * _tileSize.x, static_cast<double>(_height) * _tileSize.y);
		Vector2D window = PropertyManager.GetSettingWindowSize();
		if (not (0.0 < window.x) || not (0.0 < window.y)) {
			window = PropertyManager.GetRealWindowSize();
		}
		if (0.0 < size.x && 0.0 < size.y && 0.0 < window.x && 0.0 < window.y) {
			// マップ領域 : パネル領域 = MAP_PANEL_RATIO : 1 （ウィンドウ横幅に対する比率）。
			// マップは、横がマップ領域に合うか、縦がウィンドウサイズに一致するまでの拡大率で表示する。
			const double mapAreaWidth = window.x * MAP_PANEL_RATIO / (MAP_PANEL_RATIO + 1.0);
			const double scaleFactor = Min(mapAreaWidth / size.x, window.y / size.y);
			SetScale(scaleFactor);
			const Vector2D offset((mapAreaWidth - size.x * scaleFactor) * 0.5, (window.y - size.y * scaleFactor) * 0.5);
			SetPosition(offset);
			if (_infoPanel) {
				// パネルは盤面の変形を受けないよう逆変換し、実寸で表示する
				_infoPanel->SetPosition({ (mapAreaWidth - offset.x) / scaleFactor, -offset.y / scaleFactor });
				_infoPanel->SetScale(1.0 / scaleFactor);
				_infoPanel->SetPanelSize(window.x - mapAreaWidth, window.y);
			}
		}
	}

	void BoardBase::BuildTiles(const std::map<BoardTerrain, std::string>& textures, const std::vector<BoardTileData>& tiles) {
		const auto defaultIt = textures.find(BoardTerrain::Normal);
		const auto defaultTexture = defaultIt == textures.end() ? std::string() : defaultIt->second;
		for (int y = 0; y < _height; y++) {
			for (int x = 0; x < _width; x++) {
				const auto index = GetTileIndex(x, y);
				const auto tileData = index < static_cast<int>(tiles.size())
					? tiles[index] : BoardTileData{};
				const auto terrain = tileData.terrain;
				const auto texturePath = tileData.texture.empty() ? defaultTexture : tileData.texture;
				const auto& tile = CreateScene<BoardTile>();
				tile->SetTile(terrain, texturePath.empty() ? nullptr : LoadAsset<Texture>(texturePath), _tileSize);
				tile->SetPosition(GetTilePosition(x, y));
				_tiles.push_back(tile);
				tile->LeftPressed += [this, x, y](const MouseEvent& event) {
					if (event.just) SelectTile(x, y);
					};
				tile->RightPressed += [this](const MouseEvent& event) {
					if (event.just) CancelSelection();
					};
			}
		}
	}

	void BoardBase::BuildObjects(const std::vector<BoardObjectData>& objects) {
		for (const auto& data : objects) {
			if (data.texture.empty()) continue;
			const auto& sprite = CreateScene<Sprite2D>();
			sprite->SetTexture(LoadAsset<Texture>(data.texture));
			sprite->SetScale({ data.scaleX, data.scaleY });
			sprite->SetPosition(GetTilePosition(data.x, data.y));
			_objects.push_back(sprite);
		}
	}

	void BoardBase::BuildUnits(const std::vector<BoardUnitData>& units) {
		for (const auto& data : units) {
			const auto& unit = CreateScene<BoardUnit>();
			unit->SetUnit(data, data.texture.empty() ? nullptr : LoadAsset<Texture>(data.texture), _tileSize);
			unit->SetPosition(GetTilePosition(data.x, data.y));
			// ステータス永続化用に、ユニットごとにキャラクターを1体保持する
			const auto battleChara = CreateBattleCharacterForUnit(*unit);
			if (battleChara) {
				battleChara->Load();
				unit->SetCharacter(battleChara);
				if (const auto enemyChara = std::dynamic_pointer_cast<EnemyBattleCharacter>(battleChara)) {
					unit->SetRelativeMoveOffsets(enemyChara->GetRelativeMoveOffsets());
				}
			}
			unit->LeftPressed += [this, unit](const MouseEvent& event) {
				if (event.just) SelectUnit(unit);
				};
			unit->RightPressed += [this](const MouseEvent& event) {
				if (event.just) CancelSelection();
				};
			_units.push_back(unit);
		}
		if (_infoPanel) {
			// 味方パネルは プレア、レコ、ゾン、ゴウ の順で 2x2 表示する
			std::vector<SharedPtr<BattleCharacter>> allyCharacters;
			const BoardCharacterType order[] = { BoardCharacterType::Plea, BoardCharacterType::Reko, BoardCharacterType::Zon, BoardCharacterType::Gou };
			for (const auto type : order) {
				for (const auto& unit : _units) {
					if (unit->IsAlly() && unit->GetData().characterType == type && unit->GetCharacter()) {
						allyCharacters.push_back(unit->GetCharacter());
						break;
					}
				}
			}
			_infoPanel->SetAllies(allyCharacters);
		}
	}

	void BoardBase::SelectUnit(const SharedPtr<BoardUnit>& unit) {
		if (_winScreen || _isEnemyTurn || _isAnimating || _isCoopSelecting) return;
		if (not unit->IsAlly()) {
			if (_selectedUnit) {
				SelectTile(unit->GetData().x, unit->GetData().y);
			}
			else {
				ShowEnemyInfo(unit->GetData().x, unit->GetData().y);
			}
			return;
		}
		CancelSelection();
		_selectedUnit = unit;
		GetTile(unit->GetData().x, unit->GetData().y)->SetSelected(true);
		UpdateMoveHints();
	}

	void BoardBase::SelectTile(int x, int y) {
		if (_winScreen || _isEnemyTurn || _isAnimating || _isCoopSelecting) return;
		if (not _selectedUnit) {
			ShowEnemyInfo(x, y);
			return;
		}
		const auto movingUnit = _selectedUnit;
		if (IsMoveAllowed(*movingUnit, x, y, true)) {
			const auto targetUnit = GetUnitAt(x, y);
			const bool isEnemyEncounter = targetUnit && not targetUnit->IsAlly();
			CancelSelection();
			MoveUnit(movingUnit, x, y, true, [this, movingUnit, targetUnit, isEnemyEncounter]() {
				if (isEnemyEncounter) {
					OpenCoopSelect(movingUnit, targetUnit);
				}
				else {
					ExecuteEnemyTurn();
				}
			});
		}
	}

	void BoardBase::CancelSelection() {
		for (const auto& tile : _tiles) {
			tile->SetSelected(false);
			tile->SetMoveHint(false);
		}
		_selectedUnit.reset();
	}

	void BoardBase::UpdateMoveHints() {
		if (not _selectedUnit) return;
		for (int y = 0; y < _height; y++) {
			for (int x = 0; x < _width; x++) {
				GetTile(x, y)->SetMoveHint(IsMoveAllowed(*_selectedUnit, x, y, true));
			}
		}
	}

	void BoardBase::ClearEnemyIntentVisuals() {
		for (const auto& enemy : GetEnemyUnits()) {
			enemy->SetIntentHint(false);
		}
		for (const auto& tile : _tiles) {
			tile->SetEnemyTargetHint(false);
		}
	}

	void BoardBase::UpdateEnemyIntentVisuals() {
		ClearEnemyIntentVisuals();
		_nextEnemyAction.reset();
		if (_isEnemyTurn) return;

		_nextEnemyAction = DecideEnemyAction();
		if (_nextEnemyAction.has_value()) {
			if (_nextEnemyAction->enemy) {
				_nextEnemyAction->enemy->SetIntentHint(true);
			}
			const auto targetTile = GetTile(_nextEnemyAction->targetX, _nextEnemyAction->targetY);
			if (targetTile) {
				targetTile->SetEnemyTargetHint(true);
			}
		}
	}

	void BoardBase::ExecuteEnemyTurn() {
		_isEnemyTurn = true;
		ClearEnemyIntentVisuals();

		std::optional<BoardEnemyAction> action = _nextEnemyAction;
		_nextEnemyAction.reset();

		if (not action.has_value() || not action->enemy) {
			action = DecideEnemyAction();
		}

		if (action.has_value() && action->enemy) {
			const auto targetUnit = GetUnitAt(action->targetX, action->targetY);
			const bool isAllyEncounter = targetUnit && targetUnit->IsAlly();
			if (CanMoveTo(*action->enemy, action->targetX, action->targetY, true)) {
				MoveUnit(action->enemy, action->targetX, action->targetY, true, [this, enemy = action->enemy, targetUnit, isAllyEncounter]() {
					if (isAllyEncounter) {
						OpenCoopSelect(targetUnit, enemy);
					}
					else {
						_turn++;
						_isEnemyTurn = false;
						UpdateEnemyIntentVisuals();
					}
				});
				return;
			}
			const auto fallbackAction = DecideEnemyAction();
			if (fallbackAction.has_value() && fallbackAction->enemy && CanMoveTo(*fallbackAction->enemy, fallbackAction->targetX, fallbackAction->targetY, true)) {
				const auto fallbackTarget = GetUnitAt(fallbackAction->targetX, fallbackAction->targetY);
				const bool fallbackEncounter = fallbackTarget && fallbackTarget->IsAlly();
				MoveUnit(fallbackAction->enemy, fallbackAction->targetX, fallbackAction->targetY, true, [this, enemy = fallbackAction->enemy, fallbackTarget, fallbackEncounter]() {
					if (fallbackEncounter) {
						OpenCoopSelect(fallbackTarget, enemy);
					}
					else {
						_turn++;
						_isEnemyTurn = false;
						UpdateEnemyIntentVisuals();
					}
				});
				return;
			}
		}

		_turn++;
		_isEnemyTurn = false;
		UpdateEnemyIntentVisuals();
	}

	void BoardBase::EndTurn() {
		if (_winScreen || _isEnemyTurn || _isAnimating || _isCoopSelecting) return;
		CancelSelection();
		ExecuteEnemyTurn();
	}

	bool BoardBase::CanReachTile(const BoardUnit& unit, int targetX, int targetY) const {
		if (not IsInside(targetX, targetY)) return false;
		if (not GetTile(targetX, targetY)->IsPassable()) return false;
		const auto& data = unit.GetData();
		const auto dx = targetX - data.x;
		const auto dy = targetY - data.y;
		if (dx == 0 && dy == 0) return false;

		const auto absX = std::abs(dx);
		const auto absY = std::abs(dy);
		if (data.moveType == BoardMoveType::Adjacent) return not (1 < absX) && not (1 < absY);
		if (data.moveType == BoardMoveType::RangeTwo) return not (2 < absX) && not (2 < absY) && not IsBlockedBeyondUnit(unit, dx, dy);
		if (data.moveType == BoardMoveType::DiagonalUnlimited) {
			if (dx == 0 || dy == 0) return not (1 < absX) && not (1 < absY);
			if (absX != absY) return false;
			return IsPathClear(data.x, data.y, targetX, targetY);
		}
		if (dx != 0 && dy != 0) return false;
		return IsPathClear(data.x, data.y, targetX, targetY);
	}

	void BoardBase::OpenCoopSelect(const SharedPtr<BoardUnit>& primaryAlly, const SharedPtr<BoardUnit>& enemy) {
		_isCoopSelecting = true;
		std::vector<SharedPtr<BoardUnit>> availableAllies;
		const int battleX = enemy->GetData().x;
		const int battleY = enemy->GetData().y;
		for (const auto& unit : GetAllyUnits()) {
			if (unit != primaryAlly && CanReachTile(*unit, battleX, battleY)) {
				availableAllies.push_back(unit);
			}
		}

		_coopModal = CreateScene<CoopSelectModal>();
		_coopModal->Setup(primaryAlly, enemy, availableAllies, _tileSize, [this, primaryAlly, enemy](const std::vector<SharedPtr<BoardUnit>>& selectedCoopAllies) {
			if (_coopModal) {
				_coopModal->Free();
				_coopModal = nullptr;
			}
			_isCoopSelecting = false;
			_pendingTransition = [this, primaryAlly, enemy, selectedCoopAllies]() {
				StartBattle(primaryAlly, enemy, selectedCoopAllies);
			};
		});
	}

	void BoardBase::SetupBattle(BattleBase& battle, const SharedPtr<BoardUnit>& allyUnit, const SharedPtr<BoardUnit>& enemyUnit, const std::vector<SharedPtr<BoardUnit>>& coopAllies) {
		_battleParticipants.clear();
		auto addParticipant = [this, &battle](const SharedPtr<BoardUnit>& unit, bool isEnemy) {
			if (not unit) return;
			const auto character = CreateBattleCharacterForUnit(*unit);
			if (not character) return;
			if (isEnemy) {
				battle.AddEnemy(character);
			}
			else {
				battle.AddAlly(character);
			}
			_battleParticipants.push_back({ unit, character });
		};
		addParticipant(allyUnit, false);
		for (const auto& coopUnit : coopAllies) {
			if (coopUnit && coopUnit != allyUnit) {
				addParticipant(coopUnit, false);
			}
		}
		addParticipant(enemyUnit, true);

		// 戦闘シーンのロード完了後（各キャラの初期化後）に、Board 側の永続状態を引き継ぐ
		const auto participants = _battleParticipants;
		battle.EnteredMainScene.Connect([participants]() {
			for (const auto& [unit, character] : participants) {
				if (unit && character && unit->GetCharacter()) {
					character->RestoreState(unit->GetCharacter()->GetState());
				}
			}
		});
	}

	void BoardBase::StartBattle(const SharedPtr<BoardUnit>& allyUnit, const SharedPtr<BoardUnit>& enemyUnit, const std::vector<SharedPtr<BoardUnit>>& coopAllies) {
		_battleEnemyUnit = enemyUnit;
		_battleWarningEnemy = _nextEnemyAction.has_value() ? _nextEnemyAction->enemy : nullptr;
		_battleStartedInEnemyTurn = _isEnemyTurn;
		CommonData::LastBattleEnemyDefeated = false;
		const auto battleScene = MakeShared<BattleBase>();
		SetupBattle(*battleScene, allyUnit, enemyUnit, coopAllies);

		// 戦闘シーンの前に、戦闘開始演出シーンを挟む
		std::vector<BattleStartIconData> icons;
		auto addIcon = [this, &icons](const SharedPtr<BoardUnit>& unit, bool isEnemy) {
			if (not unit) return;
			const auto& data = unit->GetData();
			std::string texture = data.texture;
			if (texture.empty() && unit->GetCharacter()) {
				texture = unit->GetCharacter()->GetInfo().icon.string();
			}
			if (texture.empty()) return;
			const auto boardScale = GetScale().x;
			icons.push_back({
				texture,
				GetGlobalPosition() + GetTilePosition(data.x, data.y) * boardScale,
				_tileSize.x * boardScale,
				isEnemy
			});
		};
		addIcon(allyUnit, false);
		for (const auto& coopUnit : coopAllies) {
			if (coopUnit && coopUnit != allyUnit) {
				addIcon(coopUnit, false);
			}
		}
		addIcon(enemyUnit, true);

		const auto startScene = MakeShared<BattleStartScene>();
		startScene->Setup(icons, [this, battleScene]() {
			// 開始演出の後はローディングシーンを挟み、必要なアセットを
			// 読み込んでから戦闘シーンへ遷移する
			const auto loadingScene = MakeShared<BattleLoadingScene>();
			std::vector<SharedPtr<BattleCharacter>> characters;
			for (const auto& [unit, character] : _battleParticipants) {
				// バトル側のキャラはこの時点ではまだ _Load 前でスキルや情報が
				// 未設定のため、初期化済みの Board 側の永続キャラから集める
				auto source = unit ? unit->GetCharacter() : nullptr;
				if (not source) source = character;
				if (source) characters.push_back(source);
			}
			loadingScene->Setup(battleScene, characters, [this, battleScene]() {
				TransitionToBattle(battleScene);
				});
			SceneManager.TransSceneTo(loadingScene, 0.3, 0.3, false);
			});

		_bgmPlayer->FadeAndPause(0.5f);
		CreateTimer(0.6, [this, startScene]() {
			SceneManager.TransSceneTo(startScene, 0.3, 0.3, false);
		});
	}

	void BoardBase::OnReturnedFromBattle() {
		if (not _battleEnemyUnit) return;

		// 戦闘結果を Board 側の永続状態へ書き戻し、パネル表示を更新する
		for (const auto& [unit, character] : _battleParticipants) {
			if (unit && character && unit->GetCharacter()) {
				unit->GetCharacter()->RestoreState(character->GetState());
			}
		}
		_battleParticipants.clear();
		if (_infoPanel) {
			_infoPanel->ClearEnemies();
			_infoPanel->UpdateView();
		}

		const auto battleEnemy = _battleEnemyUnit;
		const auto warningEnemy = _battleWarningEnemy;
		const bool startedInEnemyTurn = _battleStartedInEnemyTurn;
		_battleEnemyUnit.reset();
		_battleWarningEnemy.reset();
		_battleStartedInEnemyTurn = false;

		const bool defeated = CommonData::LastBattleEnemyDefeated;
		CommonData::LastBattleEnemyDefeated = false;

		auto resumeFlow = [this, battleEnemy, warningEnemy, defeated, startedInEnemyTurn]() {
			if (GetEnemyUnits().empty()) {
				ShowWinScreen();
				return;
			}
			if (startedInEnemyTurn) {
				// 敵手番中に発生した戦闘が終わったので、敵手番を終えて次のターンへ
				_turn++;
				_isEnemyTurn = false;
				UpdateEnemyIntentVisuals();
				return;
			}
			if (defeated && warningEnemy && warningEnemy == battleEnemy) {
				// 警告を出していた敵を倒したので、敵は動かずに次のターンへ
				_turn++;
				UpdateEnemyIntentVisuals();
				return;
			}
			// 警告を出していた敵がまだ生きているなら、その敵が動く
			ExecuteEnemyTurn();
		};

		if (defeated) {
			RemoveEnemyUnit(battleEnemy, resumeFlow);
		}
		else {
			resumeFlow();
		}
	}

	void BoardBase::RemoveEnemyUnit(const SharedPtr<BoardUnit>& enemy, const std::function<void()>& onComplete) {
		if (not enemy) {
			if (onComplete) onComplete();
			return;
		}
		_isAnimating = true;
		enemy->SetIntentHint(false);
		PlayEnemyRemoveEffect(enemy, [this, enemy, onComplete]() {
			std::erase(_units, enemy);
			enemy->Free();
			_isAnimating = false;
			if (onComplete) onComplete();
		});
	}

	void BoardBase::PlayEnemyRemoveEffect(const SharedPtr<BoardUnit>& enemy, const std::function<void()>& onComplete) {
		if (onComplete) onComplete();
	}

	void BoardBase::ShowEnemyInfo(int x, int y) {
		if (not _infoPanel) return;
		std::vector<SharedPtr<BattleCharacter>> enemies;
		for (const auto& unit : _units) {
			if (not unit->IsAlly() && unit->GetData().x == x && unit->GetData().y == y && unit->GetCharacter()) {
				enemies.push_back(unit->GetCharacter());
			}
		}
		_infoPanel->ShowEnemies(enemies);
	}

	void BoardBase::ShowWinScreen() {
		if (_winScreen) return;
		CancelSelection();
		ClearEnemyIntentVisuals();
		_winScreen = CreateScene<BoardWinScreen>();
	}

	void BoardBase::TransitionToBattle(const SharedPtr<BattleBase>& battleScene) {
		SceneManager.TransSceneTo(battleScene, 1.0, 0.3, false);
	}

	bool BoardBase::MoveUnit(const SharedPtr<BoardUnit>& unit, int x, int y, bool allowOccupiedByTarget, const std::function<void()>& onComplete) {
		if (not IsMoveAllowed(*unit, x, y, allowOccupiedByTarget)) return false;
		_isAnimating = true;
		unit->SetBoardPosition(x, y);
		const auto& targetPos = GetTilePosition(x, y);
		const auto n = unit->position.Perpendicular().Normalized();
		const auto midPos = unit->position + (targetPos - unit->position) * 0.7 - Sign(n.y) * n * 50.0;
		const auto& tg = unit->CreateTweenGroup();
		tg->CreateTween(&unit->position, midPos, 0.35, Easing::EaseOutCubic);
		tg->CreateTween(&unit->position, targetPos, 0.35, Easing::EaseInCubic);

		const auto& tg2 = unit->CreateTweenGroup();
		tg2->CreateTween(&unit->scale, Vec2{ 0.8, 1.2 }, 0.35, Easing::EaseOutCubic);
		tg2->CreateTween(&unit->scale, Vec2{ 1.2, 0.8 }, 0.35, Easing::EaseOutCubic);
		tg2->CreateTween(&unit->scale, Vec2D_1, 0.15);

		tg->OnComplete([this, onComplete]() {
			_isAnimating = false;
			if (onComplete) onComplete();
		});
		return true;
	}

	std::vector<SharedPtr<BoardUnit>> BoardBase::GetUnitsByTeam(BoardTeam team) const {
		std::vector<SharedPtr<BoardUnit>> result;
		for (const auto& unit : _units) {
			if (unit->GetData().team == team) {
				result.push_back(unit);
			}
		}
		return result;
	}

	SharedPtr<BoardUnit> BoardBase::GetUnitAt(int x, int y) const {
		for (const auto& unit : _units) {
			if (unit->GetData().x == x && unit->GetData().y == y) {
				return unit;
			}
		}
		return nullptr;
	}

	std::vector<BoardOffset> BoardBase::GetEnemyMoveOffsets(const BoardUnit& unit) const {
		const auto& offsets = unit.GetRelativeMoveOffsets();
		if (not offsets.empty()) return offsets;

		const auto battleChara = CreateBattleCharacterForUnit(unit);
		if (const auto enemyChara = std::dynamic_pointer_cast<EnemyBattleCharacter>(battleChara)) {
			const auto enemyOffsets = enemyChara->GetRelativeMoveOffsets();
			if (not enemyOffsets.empty()) return enemyOffsets;
		}

		return {
			{ -1, -1 }, { 0, -1 }, { 1, -1 },
			{ -1,  0 },            { 1,  0 },
			{ -1,  1 }, { 0,  1 }, { 1,  1 }
		};
	}

	int BoardBase::GetManhattanDistance(int x1, int y1, int x2, int y2) const {
		return std::abs(x1 - x2) + std::abs(y1 - y2);
	}

	SharedPtr<BoardUnit> BoardBase::FindNearestAlly(int x, int y) const {
		SharedPtr<BoardUnit> nearest;
		int minDistance = 999999;
		for (const auto& ally : GetAllyUnits()) {
			const int dist = GetManhattanDistance(x, y, ally->GetData().x, ally->GetData().y);
			if (dist < minDistance) {
				minDistance = dist;
				nearest = ally;
			}
		}
		return nearest;
	}

	SharedPtr<BoardUnit> BoardBase::FindNearestEnemy(int x, int y) const {
		SharedPtr<BoardUnit> nearest;
		int minDistance = 999999;
		for (const auto& enemy : GetEnemyUnits()) {
			const int dist = GetManhattanDistance(x, y, enemy->GetData().x, enemy->GetData().y);
			if (dist < minDistance) {
				minDistance = dist;
				nearest = enemy;
			}
		}
		return nearest;
	}

	std::vector<std::pair<int, int>> BoardBase::GetReachablePositions(const BoardUnit& unit, bool allowOccupiedByTarget) const {
		std::vector<std::pair<int, int>> positions;
		if (not unit.IsAlly()) {
			const auto offsets = GetEnemyMoveOffsets(unit);
			for (const auto& offset : offsets) {
				const int targetX = unit.GetData().x + offset.dx;
				const int targetY = unit.GetData().y + offset.dy;
				if (CanMoveTo(unit, targetX, targetY, allowOccupiedByTarget)) {
					positions.push_back({ targetX, targetY });
				}
			}
			return positions;
		}

		for (int y = 0; y < _height; y++) {
			for (int x = 0; x < _width; x++) {
				if (CanMoveTo(unit, x, y, allowOccupiedByTarget)) {
					positions.push_back({ x, y });
				}
			}
		}
		return positions;
	}

	std::optional<BoardEnemyAction> BoardBase::DecideEnemyAction() {
		if (_customEnemyActionDecider) {
			return _customEnemyActionDecider(*this);
		}
		const auto enemies = GetEnemyUnits();
		if (enemies.empty()) return std::nullopt;

		const auto allies = GetAllyUnits();
		if (allies.empty()) return std::nullopt;

		// 優先度1: 移動可能マスにプレイヤーがいればそのマスに移動する(予定)
		for (const auto& enemy : enemies) {
			const auto reachable = GetReachablePositions(*enemy, true);
			for (const auto& [tx, ty] : reachable) {
				const auto occupant = GetUnitAt(tx, ty);
				if (occupant && occupant->IsAlly()) {
					return BoardEnemyAction{ enemy, tx, ty };
				}
			}
		}

		// 優先度2: 最も近いプレイヤーに近づくように移動する
		SharedPtr<BoardUnit> bestEnemy;
		int bestDist = 999999;
		std::pair<int, int> bestTarget = { -1, -1 };

		for (const auto& enemy : enemies) {
			const auto nearestAlly = FindNearestAlly(enemy->GetData().x, enemy->GetData().y);
			if (not nearestAlly) continue;

			const int currentDist = GetManhattanDistance(
				enemy->GetData().x, enemy->GetData().y,
				nearestAlly->GetData().x, nearestAlly->GetData().y
			);

			const auto reachable = GetReachablePositions(*enemy, false);
			for (const auto& [tx, ty] : reachable) {
				const int dist = GetManhattanDistance(
					tx, ty,
					nearestAlly->GetData().x, nearestAlly->GetData().y
				);
				if (dist < currentDist && dist < bestDist) {
					bestDist = dist;
					bestEnemy = enemy;
					bestTarget = { tx, ty };
				}
			}
		}

		if (bestEnemy && not (bestTarget.first < 0)) {
			return BoardEnemyAction{ bestEnemy, bestTarget.first, bestTarget.second };
		}

		// 優先度3: 近づけない場合でも、移動可能マスがあれば最初の敵を動かす
		for (const auto& enemy : enemies) {
			const auto reachable = GetReachablePositions(*enemy, false);
			if (not reachable.empty()) {
				return BoardEnemyAction{ enemy, reachable.front().first, reachable.front().second };
			}
		}

		return std::nullopt;
	}

	bool BoardBase::IsMoveAllowed(const BoardUnit& unit, int x, int y, bool allowOccupiedByTarget) const {
		if (not IsInside(x, y)) return false;
		if (not GetTile(x, y)->IsPassable()) return false;

		const auto occupant = GetUnitAt(x, y);
		if (occupant && occupant.get() != &unit) {
			if (allowOccupiedByTarget) {
				if (unit.IsAlly() == occupant->IsAlly()) {
					return false;
				}
			}
			else {
				return false;
			}
		}

		const auto& data = unit.GetData();
		const auto dx = x - data.x;
		const auto dy = y - data.y;
		if (dx == 0 && dy == 0) return false;

		// 敵の場合は相対移動マス定義を判定
		if (not unit.IsAlly()) {
			const auto offsets = GetEnemyMoveOffsets(unit);
			for (const auto& offset : offsets) {
				if (offset.dx == dx && offset.dy == dy) {
					return true;
				}
			}
			return false;
		}

		// 味方の場合は既存の MoveType 判定
		const auto absX = std::abs(dx);
		const auto absY = std::abs(dy);
		if (data.moveType == BoardMoveType::Adjacent) return not (1 < absX) && not (1 < absY);
		if (data.moveType == BoardMoveType::RangeTwo) return not (2 < absX) && not (2 < absY) && not IsBlockedBeyondUnit(unit, dx, dy);
		if (data.moveType == BoardMoveType::DiagonalUnlimited) {
			if (dx == 0 || dy == 0) return not (1 < absX) && not (1 < absY);
			if (absX != absY) return false;
			return IsPathClear(data.x, data.y, x, y);
		}
		if (dx != 0 && dy != 0) return false;
		return IsPathClear(data.x, data.y, x, y);
	}

	bool BoardBase::IsBlockedBeyondUnit(const BoardUnit& unit, int dx, int dy) const {
		// 移動先が、他のユニットから見て「そのユニットより奥側」にあるかを軸ごとに判定する。
		// 軸の値が 0 のユニットについては、その軸の移動先も 0 でなければならない。
		auto isBeyondAxis = [](int target, int blocker) {
			if (blocker == 0) return target == 0;
			if (target == 0) return false;
			if ((target < 0) != (blocker < 0)) return false;
			return not (std::abs(target) < std::abs(blocker));
		};
		const auto& data = unit.GetData();
		for (const auto& other : _units) {
			if (other.get() == &unit) continue;
			const int blockerX = other->GetData().x - data.x;
			const int blockerY = other->GetData().y - data.y;
			if (blockerX == 0 && blockerY == 0) continue;
			if (dx == blockerX && dy == blockerY) continue; // そのユニット自体のマスは対象外（占有ルールで判定）
			if (isBeyondAxis(dx, blockerX) && isBeyondAxis(dy, blockerY)) return true;
		}
		return false;
	}

	bool BoardBase::IsPathClear(int startX, int startY, int endX, int endY) const {
		const auto stepX = endX == startX ? 0 : (endX < startX ? -1 : 1);
		const auto stepY = endY == startY ? 0 : (endY < startY ? -1 : 1);
		auto x = startX + stepX;
		auto y = startY + stepY;
		while (x != endX || y != endY) {
			if (not GetTile(x, y)->IsPassable() || IsOccupied(x, y)) return false;
			x += stepX;
			y += stepY;
		}
		return true;
	}

	bool BoardBase::IsInside(int x, int y) const {
		return not (x < 0) && x < _width && not (y < 0) && y < _height;
	}

	bool BoardBase::IsOccupied(int x, int y, const BoardUnit* except) const {
		for (const auto& unit : _units) {
			if (unit.get() != except && unit->GetData().x == x && unit->GetData().y == y) return true;
		}
		return false;
	}

	int BoardBase::GetTileIndex(int x, int y) const {
		return y * _width + x;
	}

	Vector2D BoardBase::GetTilePosition(int x, int y) const {
		return _boardPosition + Vector2D(x * _tileSize.x, y * _tileSize.y);
	}

	SharedPtr<BoardTile> BoardBase::GetTile(int x, int y) const {
		if (not IsInside(x, y)) return nullptr;
		return _tiles.at(GetTileIndex(x, y));
	}

	SharedPtr<BattleCharacter> BoardBase::CreateBattleCharacterForUnit(const BoardUnit& unit) const {
		return CreateBattleCharacter(unit.GetData().characterType);
	}
}
