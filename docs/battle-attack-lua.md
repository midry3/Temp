# 攻撃演出 Lua スクリプト

攻撃演出は、スキルの `directionFile` で指定した Lua ファイルのグローバル変数
`attack` を読み込んで実行します。

```lua
attack = {
    duration = 1.0,
    actions = {
        { time = 0.0,         type = "animation", target = "actor", animation = 1 },
        { time = 0.1, type = "position_tween", target = "actor",
        position = { x = 120, y = 0 }, duration = 0.25,
        easing = "easeoutquad", restore = true },
        { time = 0.2, type = "animation", target = "actor", animation = 1,
        after = {
            position = { x = 0, y = -24 },
            offset = { x = 8, y = 0 },
            duration = 0.2,
            easing = "easeoutquad"
        }
        },
        {
            time = 0.5,
            type = "effect",
            effect = "chara/plea/effect/fire.efk",
            offset = { x = 16, y = -8 }
        },
        { time = 0.6, type = "damage", target = "target", amount = 30 },
        {
            time = 0.7,
            type = "blur_in",
            duration = 0.3,
            strength = 2.5,
            center = { x = 1000, y = 500 },
            focusRadius = 160
        },
        { time = 1.0, type = "blur_out", duration = 0.3, strength = 2.5,
            center = { x = 1000, y = 500 }, focusRadius = 160 },
        { time = 1.0, type = "animation", target = "actor", animation = 0 }
    }
}
```

## `attack`

| フィールド | 型 | 説明 |
| --- | --- | --- |
| `duration` | number | 演出全体の長さ。秒単位。 |
| `actions` | table | 時系列で実行するアクションの配列。`time` の昇順で実行されます。 |

## アクション

| フィールド | 型 | 対応する `type` | 説明 |
| --- | --- | --- | --- |
| `time` | number | すべて | 演出開始から実行するまでの時間。秒単位。 |
| `type` | string | すべて | `animation`、`position`、`position_tween`、`damage`、`effect`、`pickup`、`blur_in`、`blur_out`、`concentration_lines`、`screen_shake`、`time_scale` のいずれか。 |
| `target` | string | `animation`、`damage` | 対象。`actor` は攻撃者、`target` は攻撃対象を表します。 |
| `animation` | integer | `animation` | 対象に再生するアニメーション番号。 |
| `reverse` | boolean | `animation` | `true` の場合は逆再生します。既定値は `false` です。 |
| `loop` | boolean | `animation` | ループ再生するか指定します。省略時は `.anim` の `loop` 設定を使用します。 |
| `position` | Vector2D | `position`、`position_tween` | 対象の `AnimatedSprite2D` に設定するローカル座標。 |
| `reset` | boolean | `position`、`position_tween` | `true` の場合、攻撃演出開始時の初期位置へ戻します。`position_tween` では戻してからTweenを開始します。 |
| `restore` | boolean | `position_tween` | `true` の場合、指定位置へのTween完了後、同じ時間をかけて初期位置へ戻します。 |
| `easing` | string | `position_tween` | Tweenのイージング。`linear`、`easeinquad`、`easeoutquad`、`easeinoutquad`、`easeincubic`、`easeoutcubic`、`easeinoutcubic` が指定できます。既定値は `linear` です。 |
| `after` | table | `animation` | アニメーション終了時に実行する位置変更。`position`、`offset`、`duration`、`easing`を指定できます。 |
| `amount` | integer | `damage` | 与えるダメージ量。0 未満の場合は 0 として扱われます。 |
| `effect` | string | `effect` | 再生する Effekseer エフェクトのアセットパス。 |
| `trigger` | integer | `effect` | エフェクト再生中に送信するEffekseerトリガー番号。 |
| `triggerTime` | number | `effect` | 単一`trigger`を送信するエフェクト再生からの相対時刻。既定値は `0` 秒です。 |
| `triggers` | table[] | `effect` | 複数トリガーの `{ time = 相対秒, trigger = 番号 }` 配列。 |
| `textures` | string[] | `pickup` | ランダムに1つ選ぶテクスチャのアセットパス配列。 |
| `offset` | Vector2D | `effect`、`pickup` | 攻撃者またはPickupの基準座標に加算する位置オフセット。`{ x = 0, y = 0 }` が既定値です。 |
| `duration` | number | `pickup`、`blur_in`、`blur_out`、`concentration_lines`、`screen_shake`、`time_scale` | Pickupの表示時間または効果の長さ。秒単位。 |
| `strength` | number | `blur_in`、`blur_out`、`concentration_lines`、`screen_shake` | ぼかし、集中線、振動の強さ。ぼかしと集中線の既定値は `2.5`、指定可能範囲は `0` から `8` です。画面振動はピクセル単位で指定します。 |
| `frequency` | number | `screen_shake` | 1秒あたりの振動回数。既定値は `30` です。 |
| `center` | Vector2D | `blur_in`、`blur_out`、`concentration_lines` | フォーカスの中心座標。画面座標のピクセル単位。省略時は画面中央です。 |
| `focusRadius` | number | `blur_in`、`blur_out`、`concentration_lines` | 中心からこの距離までは鮮明または集中線なしに保ちます。ピクセル単位。既定値は `180` です。 |
| `scale` | number | `time_scale` | 時間倍率。既定値は `0.5` です。 |

`effect` アクションの `EfekseerEmitter2D` は、常に攻撃者のグローバル座標で
再生されます。`offset` を指定した場合は、その値を攻撃者の座標へ加算します。

エフェクトへトリガーを送る場合は、エフェクト再生時刻からの相対秒で指定します。

```lua
{
    time = 0.5,
    type = "effect",
    effect = "chara/reko/effect/beam.efk",
    trigger = 0,
    triggerTime = 0.3
}

{
    time = 0.5,
    type = "effect",
    effect = "chara/reko/effect/beam.efk",
    triggers = {
        { time = 0.2, trigger = 0 },
        { time = 0.8, trigger = 1 }
    }
}
```

`pickup` アクションは `Pickup` シーンを配置します。`textures` 配列からランダムに
テクスチャを1つ選び、`duration` 秒だけ表示します。`position` を指定した場合は
そのグローバル座標、省略時は `target` が示すキャラクターのグローバル座標に配置します。
`offset` を指定した場合は、配置座標へ加算します。

```lua
{
    time = 0.4,
    type = "pickup",
    target = "actor",
    position = { x = 900, y = 500 },
    offset = { x = 20, y = -10 },
    duration = 1.5,
    textures = {
        "goods/pickup_star.png",
        "goods/pickup_coin.png"
    }
}
```

`position` は対象キャラクターの `AnimatedSprite2D` のローカル位置を即時に変更します。
`position_tween` は指定した `duration` 秒をかけて変更します。`reset = true` は
攻撃演出開始時の位置へ戻す指示です。`restore = true` を指定した場合は、移動完了後に
同じ時間をかけて初期位置へ戻ります。

`animation` の `after` を指定すると、対象アニメーションの終了時に位置を変更します。
`after.position` は絶対座標、`after.offset` は現在位置への相対加算です。両方を指定した
場合は、`after.position + after.offset` が移動先になります。`after.duration` を指定すると
Tweenで移動し、省略または `0` 以下の場合は即時に移動します。ループ再生中のアニメーションは
終了しないため、`after` は実行されません。

`blur_in` は画面を徐々にぼかし、`blur_out` は画面のぼかしを徐々に解除します。
どちらも `duration` 秒で変化します。指定した `center` の近くは鮮明なまま、
遠ざかるほどぼかしが強くなります。`focusRadius` を小さくすると集中する範囲が
狭くなります。`strength` を大きくするとサンプリング範囲が広がり、より濃い
ぼかしになります。`blur_in` の完了後はぼかし状態を維持し、`blur_out` の完了時に
画面シェーダーを解除します。

`concentration_lines` は指定した中心へ向かう暖色の集中線を表示します。
`duration` の間にフェードイン・フェードアウトし、`focusRadius` の内側には表示しません。

`screen_shake` は画面全体を振動させます。`strength` は振幅、`frequency` は
振動速度を表し、時間経過とともに自動的に弱まって終了します。

`time_scale` は `duration` 秒間だけゲーム全体の時間倍率を変更します。
`duration` の計測は時間倍率の影響を受けません。例えば、次の指定は 2 秒間だけ
ゲーム時間を 0.5 倍にします。

```lua
{ time = 0.2, type = "time_scale", duration = 2.0, scale = 0.5 }
```

`value` フィールドも読み込まれますが、現在の攻撃演出では使用されません。

## キャラクターへのスキル設定

キャラクターごとのスキルは `BattleCharacter` の `SetSkills` で設定します。
`SetSkills` は派生キャラクターの `_Load` から呼び出します。

```cpp
void Plea::_Load() {
    BattleCharacter::_Load();

    SetSkills({
        { "Attack", "physical", "nature", "battle/attack.lua" },
        { "Fire", "special", "fire", "battle/fire.lua" }
    });

    Initialize();
}
```

各要素は次の `SkillInfo` フィールドを持ちます。

| フィールド | 説明 |
| --- | --- |
| `name` | スキル名。 |
| `primaryAttribute` | 第一属性。`physical` または `special`。 |
| `secondaryAttribute` | 第二属性。`fire`、`water`、`nature` のいずれか。 |
| `directionFile` | 攻撃演出 Lua ファイルのアセットパス。 |

攻撃ボタンを押すと、現在の攻撃者（味方先頭キャラクター）のスキル一覧が
スキル選択画面に渡され、その一覧から選択できます。したがって、キャラクターを
追加する場合は、そのキャラクターの `_Load` で `SetSkills` を呼び出してください。
