# 将棋ライクな盤面

盤面は `Game/Board/BoardBase` が管理し、`Assets/board/default.lua` から
盤面サイズ、タイル、跨りオブジェクト、キャラクター配置を読み込みます。

## MapEditor

`Game/Board/MapEditor` は ImGui ベースの盤面エディターです。起動すると
`Assets/board/default.lua` を読み込み、同じファイルへ保存します。ImGui の `Lua map file` 欄には
`board/default.lua` のように `Assets/` からの相対パスを指定します。

- `Tiles wide` / `Tiles high` で盤面の列数と行数、`Tile size` でマスのピクセルサイズを設定します。
- `Select tile` でマスを選ぶと、画像パスと通行可否を編集できます。
- `Paint tile` はブラシ画像と通行可否をマスへ適用します。
- `Place object` は画像パスと横幅・高さ（マス数）に従い、複数マスに跨るオブジェクトを配置します。選択後に削除やサイズ変更ができます。
- `Asset Explorer` は `Assets/` 以下の PNG テクスチャを一覧表示します。検索後に選ぶと、`Paint Tile` 中はタイルブラシへ、`Place object` 中はオブジェクト画像へ設定されます。選択中の画像は各ボタンからも適用できます。
- `Place character` は外部データのキャラクター一覧から選び、味方または敵の初期位置を置きます。味方はキャラクター種別ごとに1体、敵は同じ種別を複数配置できます。
- マウス右ボタンをドラッグするとカメラが移動し、ホイールでカーソル位置を中心に拡大・縮小します。
- `Save` は変更を Lua 盤面ファイルに書き出し、`Load` は指定したファイルを読み直します。

キャラクターの `displayName` と `icon`、マップ上のタイル画像・通行可否、オブジェクト、敵味方の配置は Lua ファイルに保存されます。キャラクターのクラス対応は `BoardCharacterType` と
`CreateBattleCharacter` に集約し、新しいクラスを追加するときは enum とファクトリーの対応を追加します（詳細な手順は [add-character-and-skill.md](add-character-and-skill.md) を参照してください）。
MapEditor は、マップの `characterCatalog` に定義がないキャラクターを組み込みの既定カタログで補うため、定義がないマップでも全キャラクターを配置できます（マップ側の定義がある場合はそちらが優先されます）。

タイルは `tiles` 配列で従来どおり地形名文字列を使うほか、個別の画像と通行可否を持つテーブルを指定できます。

```lua
tiles = {
    { terrain = "normal", passable = true, texture = "board/grass.png" },
    { terrain = "disabled", passable = false, texture = "board/wall.png" }
}
objects = {
    { texture = "board/tree.png", x = 2, y = 1, width = 2, height = 3 }
}
characterCatalog = {
    plea = { displayName = "プレア", icon = "chara/plea/idle.png" }
}
units = {
    { id = "ally_1", character = "plea", displayName = "プレア", icon = "chara/plea/idle.png", team = "ally", x = 0, y = 0 }
}
```

敵ユニットのグループはデータでは指定しません。**初期配置タイルが同じ敵同士**が自動的に同じ「敵グループ」として扱われます（後述）。初期配置が重なっていない敵は単体で扱われます。

カメラ操作や ImGui の Win32/DX11 初期化は `EnvSoundEnc` の Map1 エディター実装を参考にしています。必要な ImGui ソースは `ThirdParty/imgui-1.92.9b` に含めています。

## データ形式

```lua
board = {
    width = 9,
    height = 7,
    tileSize = 72,
    position = { x = 70, y = 55 },
    textures = {
        normal = "effect_material/white_block.png",
        disabled = "effect_material/white_block.png",
        other = "effect_material/white_block.png"
    },
    tiles = {
        "normal", "normal", "disabled"
    },
    units = {
        {
            id = "a",
            team = "ally",
            move = "diagonal_unlimited",
            x = 3,
            y = 3,
            texture = "chara/plea/idle.png"
        }
    }
}
```

`tiles` は左上から右方向、次に下方向へ並ぶ一次元配列です。
タイル属性は `normal`、`disabled`、`other` を使用できます。
`disabled` のタイルは通行できません。タイルごとに次の形式でテクスチャを
上書きすることもできます。

```lua
{ terrain = "other", texture = "effect_material/white_block.png" }
```

ユニットの `team` は `ally` または `enemy`、`move` は次のいずれかです。

| `move` | 移動範囲 |
| --- | --- |
| `diagonal_unlimited` | 周囲8マスと、障害物がない直線の斜め方向 |
| `adjacent` | 周囲8マス |
| `range_two` | 周囲24マス相当（縦横2マス以内） |
| `orthogonal_unlimited` | 障害物がない直線の縦横方向 |

### キャラ固有の移動ルール

次のキャラは、Lua の `move` 指定よりキャラ固有ルールが優先されます（`LoadBoard` で適用）。

- **プレア**: `diagonal_unlimited`（斜め無制限）。キャラ固有ルールとして Lua の `move` 指定より優先されます（エディターは `move` を常に `adjacent` で保存するため、マップによらずこの指定が必要です）。
- **ゾン**: 変更なし（Lua の `move` 指定どおり）
- **レコ**: `range_two`（周囲24マス）。ただし、味方・敵を問わず他のユニットと被った場合、そのユニットより奥側のマスは通行不可になります。判定は軸ごとで、例としてレコ (5,5) に対してプレアが (4,4) にいる場合、移動可能範囲のうち x≦4 かつ y≦4 のマス（プレア自身のマスを除く）が通行不可です。遮断判定は `BoardBase::IsBlockedBeyondUnit` で行い、`IsMoveAllowed` と `CanReachTile` の両方に適用されます。
- **ゴウ**: `orthogonal_unlimited`（将棋の飛車と同じく、障害物がない直線の縦横方向に何マスでも）

## 画面レイアウトと情報パネル

ウィンドウ横幅は マップ : 情報パネル = 2 : 1 の比率で分けられます
（比率は `BoardBase::MAP_PANEL_RATIO` 定数で変更できます）。マップは、横が
マップ領域に合うか、縦がウィンドウサイズに一致するまでの拡大率で表示されます。
盤面のスケールと配置は `BoardBase::UpdateLayout()` が計算し、情報パネル
（`Game/Board/BoardInfoPanel`）は盤面の変形を受けないよう逆変換して実寸で
配置されます。

- **上半分（味方ステータス）**: プレア、レコ、ゾン、ゴウの順で、Board専用の
  `BoardStatusPanel`（`Battle/` の `StatusPanel` を参考に、サイズとフォントを
  決め打ちにしたもの）を 2x2 で表示します。
- **下半分（敵情報）**: 味方の移動先選択中でないときに敵のいるマス（または敵ユニット）を
  クリックすると、そのマスにいる全ての敵を Board専用の `BoardEnemyStatusPanel`
  （`Battle/` の `EnemyStatsPanel` を参考にしたもの）で表示します。
  敵のいないマスをクリックすると表示はクリアされます。

## ステータスの永続化（Board単位）

各ユニットは `BoardUnit::GetCharacter()` で永続キャラクター（`BattleCharacter`）を
1体保持し、ステータスパラメーターの変化はこの状態に蓄積されます。

- 戦闘開始時: `SetupBattle` が戦闘用キャラクターを生成し、戦闘シーンのロード完了後
  （`EnteredMainScene`）に永続状態を `BattleCharacter::RestoreState` で引き継ぎます。
- 戦闘終了後: 盤面復帰時（`OnReturnedFromBattle`）に戦闘側の最終状態を永続キャラクターへ
  書き戻します。HP・MP の増減や `stat_changes` による ATK などの変化が、次の戦闘にも
  そのまま引き継がれます。
- 情報パネルの表示もこの永続状態を参照するため、戦闘後の変化がそのまま反映されます。

## サウンド

選択できるボタンには、クリック時に選択SEが鳴ります。SEのパスは
`Game/Data/SoundEffects.h` の `SELECT_SE` 定数（仮で `sound/one_shot/select_2.oga`）に
集約されており、差し替えはこの定数の変更だけで済みます。各ボタンは
`CreateSelectSePlayer` で `SoundPlayer` を子に持ち、`LeftPressed` で再生します。

対象: 盤面タイル、盤面ユニット、共闘選択の戦闘開始ボタンと参戦候補ボタン、
戦闘画面のコマンドボタン（攻撃）と技選択ボタン。
対象外: 勝利画面の全画面ボタンや共闘選択のブラインド用ブロッカーなど、
ボタンらしいボタンではないもの。

## ターン進行と操作

盤面は **1ターン1キャラ移動制**（プレイヤー手番 → 敵手番 → プレイヤー手番... の交互進行）です。

1. **プレイヤー手番**:
   - 味方ユニットを左クリックして選択し、移動先のタイルをクリックして移動します。
   - 選択中は移動可能なタイルが白く点滅します。右クリックすると選択をキャンセルできます。
   - **相手の動きの予告**: プレイヤー手番中、次に動く敵のアイコンが**赤く点滅**し、その敵の移動予定先のマスも**赤く点滅**して表示されます。
   - 味方1体が移動を完了すると、自動的に敵の手番へ移行します。
2. **敵手番**:
   - 予告されていた敵ユニットが予定マスへ移動します。
   - 敵の移動アニメーション完了後、ターンが進み（_turn++）、再びプレイヤーの手番へ戻り、次の敵の動きが赤点滅で予告されます。
3. **ターンのスキップ**:
   - BoardBase::EndTurn() を呼ぶと、味方を移動させずに敵手番を実行してターンを進められます。

## 敵の移動可能マスと移動決定ロジック

敵の動きは「相対移動可能マス」と「移動先決定ロジック」に分離して設計されています。

### 1. 相対移動可能マスの指定（Enemy側）
各敵キャラクターのクラス（EnemyBattleCharacter 派生クラス）で GetRelativeMoveOffsets() をオーバーライドし、自身からの相対座標オフセット（BoardOffset）のリストを返します。

```cpp
// 例: Slime.cpp（上下左右4マス）
std::vector<BoardOffset> Slime::GetRelativeMoveOffsets() const {
    return {
        { 0, -1 },
        { -1, 0 }, { 1, 0 },
        { 0, 1 }
    };
}
```

デフォルトの EnemyBattleCharacter::GetRelativeMoveOffsets() は周囲8マスです。

### 2. 移動先を決定するロジック（Mapクラス側）
BoardBase またはその派生Mapクラス（Map1 など）で移動先決定ロジックを記述します。
仮想関数 DecideEnemyAction() のオーバーライド、または SetEnemyActionDecider() による関数の注入が可能です。

Mapクラス側にはロジック記述用の便利なヘルパー関数が提供されています：
- GetAllyUnits() / GetEnemyUnits(): 盤面上の味方・敵ユニット一覧を取得
- GetUnitAt(int x, int y): 指定座標のユニットを取得
- GetReachablePositions(const BoardUnit& unit, bool allowOccupiedByTarget): ユニットが移動可能なマス座標一覧を取得
- FindNearestAlly(int x, int y) / FindNearestEnemy(int x, int y): 最も近いユニットを検索
- GetManhattanDistance(int x1, int y1, int x2, int y2): マンハッタン距離を計算
- CanMoveTo(const BoardUnit& unit, int x, int y, bool allowOccupiedByTarget): 移動可否を判定

#### デフォルトロジックの動作
1. **優先度1**: 移動可能マスにプレイヤー（味方）がいれば、そのマスに移動（突撃予定）
2. **優先度2**: 最も近いプレイヤーとの距離が縮まるマスへ移動
3. **優先度3**: 近づけない場合でも、空いている移動可能マスがあれば移動

## 敵グループ

初期配置タイルが同じ敵同士は、ひとかたまりのグループとして振る舞います（グループの区別は初期配置でのみ決まり、途中で同じタイルに集まってもグループにはなりません）。

- **盤面での移動**: 敵手番にグループの敵（リーダー）が動くと、他のメンバーも同じ変位で同時に移動し、相対配置を保ちます。移動先が盤外・通行不可・グループ外のユニットで塞がれているメンバーは、その場に残ります。動きの予告（赤点滅）はメンバー全員に出ます。
- **戦闘**: グループの敵と戦闘になると、グループ全員が敵として戦闘に参加します。勝利するとグループ全員が盤面から消去されます（警告を出していた敵がグループに含まれていた場合も、グループごと倒した扱いになります）。
- **エディター**: グループにしたい敵同士は、同じタイルに重ねて配置します。また、タイル選択時にはそのタイルのオブジェクトとキャラクターを削除できます。

## 複数マス占有ユニット

ユニットは本体のマス以外に複数のマスを占有できます（大型ユニットなど）。

- **データ形式**: 盤面 Lua の `units` 要素に `tiles = { { x = dx, y = dy }, ... }`（本体からの相対オフセット）を書きます。本体のマスと `tiles` の全マスが占有マスです。
- **戦闘**: 占有マスのどれかに入る（本体・占有マス同士が重なる場合を含む）と戦闘が発生します。敵情報パネルや共闘アイコンも占有マス全体を対象にします。
- **表示**: 2マス以上を占有するユニットの占有マスは、盤面上で青系に点滅します。
- **移動**: 移動すると占有マスも一緒に動きます。移動先では占有範囲全体が盤内・通行可でなければならず、味方同士・敵同士の占有マスは重なれません（相手陣営との重なりはエンカウントとして許可されます）。経路判定やグループ移動でも占有範囲全体で判定します。
- **エディター**: タイル選択時のキャラクター欄にある「占有マスを編集」を押すと編集モードになり、タイルをクリックするたびにそのキャラクターの占有マスを追加／削除できます（本体のマスと、他のキャラクターが使っているマスは指定できません）。占有マスは半透明の小さいアイコンで表示されます。味方を再配置すると占有マスも同じだけずれて引き継がれます。

## 戦闘発生とシーン遷移（エンカウント）

敵ユニットが存在するマスへの移動が可能です。味方ユニットが敵のいるマスへ移動（または敵ユニットが味方のいるマスへ移動）すると、移動アニメーション完了後に「共闘選択」を経て戦闘（BattleBase）が発生します。

### 共闘アイコン

盤面上では、敵がいるタイルの上部に、そのマスへ到達可能な味方のミニアイコンが横並びで表示されます。到達可能な味方は、その敵との戦闘に共闘で参加できる味方です。

- 同じタイルに複数の敵がいる場合は、まとめて1行だけ表示されます。
- アイコンはタイル幅に収まるよう自動で縮小されます。サイズ比率と上端の余白は `BoardBase` の `COOP_ICON_SIZE_RATIO` / `COOP_ICON_TOP_MARGIN` で変更できます。
- 味方・敵の移動や戦闘後の敵消去のたびに再計算されます。

### 共闘システム

戦闘が始まる前に、条件を満たした味方と協力して戦う「共闘」対象の選択フェーズに入ります。ポップアップモーダルではなく、**マップ上で直接味方をクリックして選択するオーバーレイUI**として完結しています。

1. **発生タイミング**:
   - 味方ユニットが敵のいるマスへ侵入した時
   - 敵ユニットが味方のいるマスへ侵入してきた時
2. **共闘可能条件**:
   - 「現在位置から戦闘発生マスに到達可能であること (`CanReachTile`)」
   - 移動可能範囲内であり、かつ直線移動等の経路上に障害物（壁や他ユニット）がないこと（※戦闘発生マス自体の占有は除外して到達判定）
3. **マップ上での共闘選択UI (`CoopSelectModal`)**:
   - 画面全体を黒の薄いブラインド（Alpha 165）で覆い、無関係な盤面や背後へのクリックを遮断します。
   - **交戦マス**: ブラインドの上に赤くパルス発光する枠線、交戦アバター（メイン味方 vs 敵）、および「【交戦マス】」バッジが表示されます。
   - **共闘候補ユニット**: マップ上の位置そのままにブラインドの上に浮き上がり、水色の枠線と「参戦可」バッジで強調表示されます。
   - **マップ上での選択操作**: マップ上の候補ユニットを直接クリックすることで、「★ 参戦中!」（金色パルス枠・緑背景）と「参戦可」がトグル切り替えされます（0人選択＝単独戦闘も可能）。
   - **戦闘開始ボタン**: 画面下部に「戦闘開始 (単独)」または「戦闘開始 (共闘 N人)」が表示され、クリックで戦闘シーンへ遷移します。

### 処理の導線と自動登録
1. **エンカウント判定と共闘選択**:
   - `SelectTile()` または `ExecuteEnemyTurn()` で移動先に相手チームのユニットが存在する場合、移動完了後に `OpenCoopSelect(primaryAlly, enemy)` が呼び出されます。
2. **キャラクターの自動登録 (SetupBattle)**:
   - `BoardBase::SetupBattle(battle, allyUnit, enemyUnit, coopAllies)` が自動的に呼び出されます。
   - `_allies`: ぶつかったメイン味方ユニット（`allyUnit`）に加え、共闘UIで選択された味方ユニット群（`coopAllies`）から `CreateBattleCharacterForUnit()` で `BattleCharacter` を生成して登録します。
   - `_enemies`: 接触した敵ユニットから `BattleCharacter` を生成して登録します。
3. **キャラクター配置とシーン遷移 (StartBattle, TransitionToBattle)**:
   - `battle->PutCharacters()` が実行されてキャラクターがグリッド上に配置されます。
   - 共闘選択が終わると、まず戦闘開始演出シーン (`Game/Battle/BattleStartScene`) へ遷移し、演出の完了後はローディングシーン (`Game/Battle/BattleLoadingScene`) を挟んで、`TransitionToBattle` で戦闘シーンへ遷移します。
   - 戦闘開始演出（暫定）: 白背景に、戦う敵味方のアイコンを Board と同じ位置・大きさで表示 → 味方は左、敵は右へ移動 → 一度中央まで互いに移動 → 弾かれたように左右へ戻る → 戦闘シーンへ。アイコンの Board 上の位置・大きさは `StartBattle` がウィンドウ座標に変換して `BattleStartScene::Setup` に渡します。
   - ローディングシーンでは、戦闘参加キャラ（敵味方全員）のアイコンと、全スキル分の演出アセット（エフェクト、Pickupテクスチャ、サウンド）をあらかじめ読み込みます。読み込みは戦闘シーンの `BattleBase::LoadAsset` 経由で行い、対象パスの収集は `BattleAttack::CollectSkillAssets` が行います。バトル側のキャラはこの時点ではまだ `_Load` 前でスキルが未設定のため、収集には初期化済みの Board 側の永続キャラを使います。汎用部分は `Game/LoadingScene`（タスクをフレームに分散して処理し、進捗を表示）に切り出されており、`BattleLoadingScene` がそれを継承しています。
   - （※Tween完了時コールバック内での即時シーン差し替えによる参照解放クラッシュを防ぐため、`_pendingTransition` を介して更新フレームの安全なタイミングでシーン遷移が実行されます）
   - 必要に応じて `BoardBase` の派生クラスで `SetupBattle` や `TransitionToBattle` をオーバーライドして挙動をカスタマイズできます。

## 戦闘終了後の盤面復帰処理

戦闘シーンから盤面に戻ると（`BoardBase::EnteredMainScene` を契機に
`OnReturnedFromBattle()` が呼ばれます）、戦闘結果に応じて次の処理が行われます。
戦闘結果の受け渡しは `StartBattle()` が戦闘相手の敵ユニットと戦闘開始時点の
予告敵・手番状態を記録し、`BattleBase` が `Won` 状態に入ったときに
`CommonData::LastBattleEnemyDefeated` を `true` にすることで行います。

1. **戦った敵を倒した場合**: その敵ユニットを盤面から消去します。
2. **盤上の敵が0になった場合**: 勝利画面（`Game/Board/BoardWinScreen`）を表示します。`Battle/WinScreen` を参考にしたもので、シーン遷移部分はコメントアウトしてあります（表示中は盤面操作を受け付けません）。
3. **敵手番中に発生した戦闘だった場合**: 敵手番を終え、ターンを進めて新しい予告を作り、プレイヤーの手番に戻ります。
4. **予告を出していた敵を倒した場合**（プレイヤー手番側で発生した戦闘）: 敵は動かず、ターンを進めて新しい予告を作り、プレイヤーの手番になります。
5. **それ以外の場合**: 予告を出していた敵がまだ生きているので、`ExecuteEnemyTurn()` でその敵が動きます。

### 敵消去時の演出の差し込み

消去は `BoardBase::RemoveEnemyUnit(enemy, onComplete)` が担当し、実際の消去
（`_units` からの除去と `Free()`）は演出完了後に行われます。演出中は
`_isAnimating` で盤面操作がブロックされます。

既定の演出は「なし」（即座に完了）です。演出を入れる場合は、派生クラスで
`PlayEnemyRemoveEffect(enemy, onComplete)` をオーバーライドし、演出が終わった
タイミングで `onComplete` を呼んでください。

```cpp
void Map1::PlayEnemyRemoveEffect(const SharedPtr<BoardUnit>& enemy, const std::function<void()>& onComplete) {
    // 例: フェードアウトさせてから消去する
    auto tween = enemy->CreateTweenGroup();
    tween->CreateTween(&enemy->alpha, 0, 0.5);
    tween->OnComplete(onComplete);
}
```
