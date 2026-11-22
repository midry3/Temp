# キャラクター（味方・敵）および技の追加手順

本作（`OnBoard`）において、新しい味方キャラクター、敵キャラクター、および敵の技を追加・実装する際の手順書です。

---

## 1. 味方キャラクターの追加手順

### 1-1. アセットの準備
`Assets/chara/<キャラクター名>/` フォルダを作成し、必要な画像やアニメーション定義を配置します。

- **スプライト画像**:
  - `idle.png`（待機アニメーション）
  - `attack.png`（攻撃アニメーション）
  - 必要に応じてダメージ、詠唱、アイコン画像などを配置。
- **アニメーション定義ファイル (`.anim`)**:
  - 画像ファイルと同じ場所に `<画像名>.anim`（JSON形式）を配置します。
  ```json
  {
      "rows": 1,
      "columns": 7,
      "fps": 8,
      "scale": 4,
      "loop": true
  }
  ```
- **アイコン画像**:
  - `icon.png`（盤面やUI表示用のアイコン）

### 1-2. キャラクタークラスの作成
`Game/Battle/BattleCharacter/` 配下にヘッダーとソースコードを作成し、`BattleCharacter` を継承します。

- **ヘッダー (`Game/Battle/BattleCharacter/<クラス名>.h`)**:
  ```cpp
  #pragma once
  #include "BattleCharacter.h"

  namespace Game {
      namespace ExampleAnimation {
          enum {
              BATTLE_CHARACTER_REQUIRED_ANIMATION
          };
      }

      class ExampleAlly : public BattleCharacter {
      private:
          static constexpr int
              DEFAULT_MAX_HP = 100,
              DEFAULT_MAX_MP = 50,
              DEFAULT_ATK = 10,
              DEFAULT_DEF = 5,
              DEFAULT_SPEED = 7;

      protected:
          void _Load() override;
      };
  }
  ```

- **ソースコード (`Game/Battle/BattleCharacter/<クラス名>.cpp`)**:
  ```cpp
  #include "ExampleAlly.h"

  namespace Game {
      void ExampleAlly::_Load() {
          BattleCharacter::_Load();

          SetInfo({
              "キャラクター表示名",
              {
                  { ExampleAnimation::Idle, "chara/example/idle.png" },
                  { ExampleAnimation::Attack, "chara/example/attack.png" }
              }
          });

          SetStats({
              DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_SPEED
          });

          SetSkills({
              { "スキル名1", "special", "fire", "chara/example/attack/fire.lua" },
              { "スキル名2", "physical", "normal", "" }
          });

          Initialize();
      }
  }
  ```

### 1-3. `BoardCharacterType` 列挙型への追加
`Game/Board/BoardTypes.h` の `BoardCharacterType` に識別子を追加します。

```cpp
enum class BoardCharacterType {
    Plea,
    Reko,
    Zon,
    Gou,
    Slime,
    ExampleAlly // 追加
};
```

### 1-4. ファクトリーへの登録
`Game/Battle/BattleCharacter/BattleCharacterFactory.cpp` に対応を追加します。

1. ヘッダーのインクルード:
   ```cpp
   #include "ExampleAlly.h"
   ```
2. `BoardCharacterTypeKey` にキー文字列を追加（マップLuaやIDで使用）:
   ```cpp
   case BoardCharacterType::ExampleAlly: return "example_ally";
   ```
3. `ParseBoardCharacterType` に文字列からのパース処理を追加:
   ```cpp
   if (key == "example_ally") return BoardCharacterType::ExampleAlly;
   ```
4. `CreateBattleCharacter` にインスタンス生成処理を追加:
   ```cpp
   case BoardCharacterType::ExampleAlly: return MakeShared<ExampleAlly>();
   ```

### 1-5. プロジェクトへの登録と盤面配置
- `OnBoard.vcxproj` および `OnBoard.vcxproj.filters` に新規作成した `.h` と `.cpp` を追加します。
- 盤面 Lua ファイル（`Assets/board/*.lua`）の `characterCatalog` および `units` に登録するか、`MapEditor` 上で配置します。

---

## 2. 敵キャラクターの追加手順

### 2-1. アセットの準備
`Assets/enemy/<敵の名前>/` フォルダを作成し、スプライト画像と `.anim` ファイルを配置します。

- 例: `Assets/enemy/slime/idle.png` および `idle.png.anim`

### 2-2. 敵クラスの作成
`Game/Battle/BattleCharacter/Enemy/` 配下にヘッダーとソースコードを作成し、`EnemyBattleCharacter` を継承します。

- **ヘッダー (`Game/Battle/BattleCharacter/Enemy/<クラス名>.h`)**:
  ```cpp
  #pragma once
  #include "EnemyBattleCharacter.h"

  namespace Game {
      namespace ExampleEnemyAnimation {
          enum {
              BATTLE_CHARACTER_REQUIRED_ANIMATION
          };
      }

      class ExampleEnemy : public EnemyBattleCharacter {
      private:
          static constexpr int
              DEFAULT_MAX_HP = 80,
              DEFAULT_MAX_MP = 20,
              DEFAULT_ATK = 8,
              DEFAULT_DEF = 3,
              DEFAULT_SPEED = 5;

      protected:
          void _Load() override;

      public:
          [[nodiscard]] std::optional<SkillInfo> SelectSkill(const SharedPtr<BattleCharacter>& target = nullptr) override;
      };
  }
  ```

- **ソースコード (`Game/Battle/BattleCharacter/Enemy/<クラス名>.cpp`)**:
  ```cpp
  #include "ExampleEnemy.h"
  #include <Math/Utils.h>

  namespace Game {
      void ExampleEnemy::_Load() {
          EnemyBattleCharacter::_Load();

          SetInfo({
              "敵表示名",
              {
                  { ExampleEnemyAnimation::Idle, "enemy/example/idle.png" },
                  { ExampleEnemyAnimation::Attack, "enemy/example/idle.png" },
                  { ExampleEnemyAnimation::Damage, "enemy/example/idle.png" },
                  { ExampleEnemyAnimation::Death, "enemy/example/idle.png" }
              }
          });

          SetStats({
              DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_SPEED
          });

          // 技の選択肢を登録
          SetSkills({
              { "たいあたり", "physical", "normal", "" },
              { "カサカサ突進", "special", "normal", "" }
          });

          // 攻撃時のカサカサ演出パラメータ（幅, 往復回数, 1往復にかける秒数）
          SetRustleParams(20.0, 4, 0.12);

          Initialize();
      }

      // プログラムによる技選択ロジック
      std::optional<SkillInfo> ExampleEnemy::SelectSkill(const SharedPtr<BattleCharacter>& target) {
          const auto& skills = GetSkills();
          if (skills.empty()) return std::nullopt;

          // 例: ランダムに技を選ぶ
          const int count = static_cast<int>(skills.size());
          const int index = Clamp(GetRandomInt(0, count), 0, count - 1);
          return skills[index];
      }
  }
  ```

### 2-3. `BoardCharacterType` およびファクトリーへの登録
味方と同様に以下の登録を行います：
1. `Game/Board/BoardTypes.h` の `BoardCharacterType` に enum 値を追加。
2. `Game/Battle/BattleCharacter/BattleCharacterFactory.cpp` にヘッダーをインクルードし、`BoardCharacterTypeKey`、`ParseBoardCharacterType`、`CreateBattleCharacter` に分岐を追加。
3. `OnBoard.vcxproj` / `OnBoard.vcxproj.filters` にファイルを追加。

---

## 3. 敵の技（スキル）の追加とカスタマイズ

### 3-1. 技データ構造（`SkillInfo`）
各技は以下のフィールドを持つ構造体です：

| フィールド | 型 | 説明 |
| --- | --- | --- |
| `name` | `std::string` | 技の表示名（例: `"たいあたり"`） |
| `primaryAttribute` | `std::string` | 主属性（`"physical"`, `"special"` など） |
| `secondaryAttribute` | `std::string` | 副属性（`"normal"`, `"fire"`, `"water"`, `"nature"` など） |
| `directionFile` | `std::string` | 演出スクリプトのパス（省略・空文字時はカサカサ演出などの簡易演出） |

### 3-2. 技の登録方法
- クラスの `_Load()` 内で `SetSkills({ ... })` を呼び出して複数登録します。
- 後から動的に追加する場合は `enemy->AddSkill({ ... })` を使用します。

### 3-3. プログラムによる技選択ロジックの実装
敵の技選択は、以下の2通りの方法で柔軟にカスタマイズできます：

#### 方法A: クラス内で `SelectSkill()` をオーバーライドする
敵ごとに固有の思考パターンを持たせたい場合に適しています。
```cpp
std::optional<SkillInfo> BossEnemy::SelectSkill(const SharedPtr<BattleCharacter>& target) {
    const auto& skills = GetSkills();
    if (skills.empty()) return std::nullopt;

    // HPが半分未満なら強力な技を選択
    if (GetCurrentHP() < GetState().maxHP / 2) {
        return skills.back();
    }
    return skills.front();
}
```

#### 方法B: 外部から `SetSkillSelector()` で関数オブジェクトを渡す
インスタンスごとに異なるAIロジックを割り当てたい場合に適しています。
```cpp
auto enemy = MakeShared<Slime>();
enemy->SetSkillSelector([](const EnemyBattleCharacter& self, const SharedPtr<BattleCharacter>& target) -> std::optional<SkillInfo> {
    const auto& skills = self.GetSkills();
    if (skills.empty()) return std::nullopt;
    // 任意の条件で選択
    return skills[0];
});
```

### 3-4. 技の実行と攻撃演出
敵の行動順になると、`ExecuteTurn(target, onComplete)` が呼び出され、以下の流れで技が実行されます：

1. `SelectSkill(target)` により技が1つ選択されます。
2. **演出の実行**:
   - **`directionFile` が指定されている場合**:
     指定された Lua スクリプトが読み込まれ、`BattleAttack` によるタイムライン演出が再生されます。
   - **`directionFile` が空の場合**:
     敵キャラクターが左右（X方向）にカサカサと振動する `PlayRustleAnimation()` 演出が再生され、演出完了時にターゲットへダメージ（自身の攻撃力 `atk`）が適用されます。
3. 演出完了後、`TurnFinished` シグナルおよびコールバック `onComplete` が呼び出されます。

### 3-5. カサカサ演出の調整
カサカサ演出は、キャラクター単位および Lua スクリプト単位で自由に設定可能です。

- **C++ でのパラメータ調整**:
  `SetRustleParams(width, roundTrips, durationPerTrip)`
  - `width`（`double`）: 左右の振れ幅（ピクセル単位）
  - `roundTrips`（`int`）: 往復回数（例: 3回、4回）
  - `durationPerTrip`（`double`）: 1往復にかける秒数（既定値: `0.16` 秒）
  - 演出は `offset` を Tween し、終了時に確実に `offset = { 0, 0 }` に復帰します。

- **Lua 演出スクリプトからの呼び出し**:
  タイムラインスクリプト内で `type = "rustle"` を指定することで、攻撃シーケンスの一部としてカサカサ演出を組み込めます。
  ```lua
  {
      time = 0.2,
      type = "rustle",
      target = "actor",
      width = 24.0,
      roundTrips = 4,
      durationPerTrip = 0.12
  }
  ```
