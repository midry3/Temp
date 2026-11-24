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

## 技の対象（SkillInfo）

各技には対象の属性があります（`SkillInfo` の `targetScope` / `targetSide`）。

- `targetScope`：`Single`（単体）または `All`（全体）。既定値は `Single` です。
- `targetSide`：`Enemy`（敵）、`Ally`（味方）、または `Self`（自身）。既定値は `Enemy` です。`Self` の場合、対象は行動者自身のみとなり、対象選択はスキップされます。
- `description`：技の説明文（末尾のフィールド）。スキル選択パネルの説明欄に表示されます。

単体対象の技を選ぶと対象選択画面になります。対象候補の情報パネル（味方は左上、敵は右上）以外は薄く暗くなり、候補パネルは縁取りと発光点滅でハイライトされます（色や点滅の調整値は `TargetSelect` の `Highlight` 系定数）。パネルをクリックして対象を選びます。候補が1体だけの場合は選択をスキップして自動でその対象になります。全体対象の技は選択なしで、対象陣営の生存者全員が対象になります。対象候補は生存しているキャラのみです。

演出側では `target = "target"` が、単体対象では選んだ相手、全体対象では対象全員を指します（`damage`、`animation`、`position`、`position_tween`、`rustle`、`stat_changes` に適用）。

## 行動順と敵の行動

1ターンの流れは、全味方のスキル選択 → 行動キューを構築 → キュー順に演出を実行、です。行動順は `speed` の大きい順で、同速の場合は味方を優先します。行動前に行動者が死亡している場合や、対象が全滅している場合はその行動を飛ばします。敵全滅で勝利（Won）、味方全滅で敗北（Lost）になります。

コマンド選択には ATK のほかに「何もしない」「降参」「戻る」があります。「何もしない」と「降参」は ATK と縦に並べて表示します（テクスチャはどちらも `ui/atk.png` で仮置き）。「何もしない」を選んだキャラはそのターン行動しません。「降参」は敗北（Lost）として扱います。「戻る」は降参の右下あたりに置き、二番目以降のキャラの選択時にだけ有効で、押すと一つ前のキャラの選択を取り消してそのキャラのコマンド選択に戻ります。スキル選択にも「戻る」ボタンがあり、押すとそのキャラのコマンド選択に戻ります。対象選択にも「戻る」ボタンがあり、押すとスキル選択に戻ります。戻るボタンは全画面で `ui/back.png`（0.7倍スケール）に統一されています。

敵の行動はAIが決めます：全スキルからランダムに1つ選び（敵はMPを無視します）、単体対象なら対象候補からランダムに1体、全体対象なら候補全員を対象にします。`targetSide` は行動者から見た陣営として解釈されます（敵にとっての `Enemy` は味方側）。

攻撃演出の間は、攻撃者と対象以外のキャラクターが薄黒になります（元の色味に `ATTACK_DIM_RATE` を掛けるため、レア種の赤系 modulate などの固有の色味は演出後にそのまま戻ります）。演出が終わると色味は復元され、味方にはフォーメーション側の薄黒が改めて適用されます。

キャラクターが死亡すると、そのキャラクターの情報パネル（味方はステータスパネル、敵は敵情報パネル）が薄黒になります。死亡した味方のコマンド選択は飛ばされ、行動キューにも入りません。参加した味方が全滅すると敗北（Lost）になり、敗北演出（`LoseScreen`）を経て Board へ戻ります（盤面側の処理は board.md の「戦闘終了後の盤面復帰処理」を参照）。

味方は横一列に並びます（右端が最前列）。コマンド選択時は選択順に並び、選択が終わったキャラはアニメーションで後ろへ回り、次のキャラが最前列に来ます。攻撃開始時は味方の行動順に並び替わり、自分の行動が終わると同様に後ろへ回ります。コマンド選択に戻ると再び選択順に並び替わります。どちらの場面でも、最前列以外の味方には薄黒がかかります（`modulate` は子シーンへ継承されないため、`BattleCharacter::SetModulateAll` で子孫全体に適用します）。間隔やアニメーション時間、暗さは `BattleBase` の `ALLY_SPACING` / `FORMATION_ANIMATION_TIME` / `FORMATION_DIM_VALUE` で調整できます。最前列かどうかは `BattleCharacter::OnFormationFront` で各キャラに通知され、レコはこれを使って最前列になるまでAlphaの透明度を50にしています。

## キャラクターのシグナル

`BattleCharacter` には次のシグナルがあります（いずれも引数なし）。

- `AttackStarted`：そのキャラが攻撃演出を開始したとき（`BattleAttack::Start` で発火）
- `Damaged`：そのキャラがダメージを受けたとき（`ApplyDamage` で実際にダメージが入った場合）
- `Died`：そのキャラが死亡したとき（HPが0になった瞬間に1回だけ）
- `StatChanged`：ステータスが変化したとき

被弾時は標準で赤の点滅（`modulate` の切り替え）が再生されます。また、キャラクターのスプライトはテクスチャの左上ではなく下端がキャラの位置に合うように表示されます（アニメーションやフレームの大きさに追随して自動で調整されます）。

## アクション

| フィールド | 型 | 対応する `type` | 説明 |
| --- | --- | --- | --- |
| `time` | number | すべて | 実行するまでの時間。秒単位。通常は演出開始からの時間、`finally = true` の場合は演出終了（`duration` 到達）からの時間になります。 |
| `finally` | boolean | すべて | `true` の場合、そのアクションは演出終了後の処理として実行されます。既定値は `false` です。詳細は「演出終了後の処理（finally）」を参照してください。 |
| `type` | string | すべて | `animation`、`position`、`position_tween`、`damage`、`effect`、`pickup`、`sound`、`screen_flash`、`blur_in`、`blur_out`、`concentration_lines`、`screen_shake`、`time_scale` のいずれか。 |
| `target` | string | `animation`、`damage`、`position`、`position_tween`、`rustle` | 対象。`actor` は攻撃者、`target` は攻撃対象を表します。単体対象スキルでは選んだ相手、全体対象スキルでは対象全員に適用されます。 |
| `animation` | integer | `animation` | 対象に再生するアニメーション番号。 |
| `reverse` | boolean | `animation` | `true` の場合は逆再生します。既定値は `false` です。 |
| `loop` | boolean | `animation` | ループ再生するか指定します。省略時は `.anim` の `loop` 設定を使用します。 |
| `position` | Vector2D | `position`、`position_tween` | 対象の `AnimatedSprite2D` に設定するローカル座標。 |
| `reset` | boolean | `position`、`position_tween` | `true` の場合、攻撃演出開始時の初期位置へ戻します。`position_tween` では戻してからTweenを開始します。 |
| `restore` | boolean | `position_tween` | `true` の場合、指定位置へのTween完了後、同じ時間をかけて初期位置へ戻します。 |
| `easing` | string | `position_tween` | Tweenのイージング。`linear`、`easeinquad`、`easeoutquad`、`easeinoutquad`、`easeincubic`、`easeoutcubic`、`easeinoutcubic` が指定できます。既定値は `linear` です。 |
| `after` | table | `animation` | アニメーション終了時に実行する位置変更。`position`、`offset`、`duration`、`easing`を指定できます。 |
| `amount` | integer | `damage` | 与えるダメージ量。実際のダメージは、技の第一属性に応じて対象の防御を引いた値になります（`physical` の場合は DEF、`special` の場合は MDEF）。防御を引いた後も最低 10 ダメージは保証されます。 |
| `effect` | string | `effect` | 再生する Effekseer エフェクトのアセットパス。 |
| `targetOrigin` | boolean | `effect` | `true` の場合、エフェクトの原点を攻撃者ではなく対象の位置にします。単体対象スキルのときのみ有効です（全体対象では無視され、攻撃者の位置になります）。既定値は `false` です。 |
| `trigger` | integer | `effect` | エフェクト再生中に送信するEffekseerトリガー番号。 |
| `triggerTime` | number | `effect` | 単一`trigger`を送信するエフェクト再生からの相対時刻。既定値は `0` 秒です。 |
| `triggers` | table[] | `effect` | 複数トリガーの `{ time = 相対秒, trigger = 番号 }` 配列。 |
| `textures` | string[] | `pickup` | ランダムに1つ選ぶテクスチャのアセットパス配列。 |
| `sound` | string | `sound` | 再生するサウンドのアセットパス（`Assets` からの相対パス）。 |
| `volume` | number | `sound` | 音量。`0` から `1` の範囲に丸められます。既定値は `1` です。 |
| `offset` | Vector2D | `effect`、`pickup` | 攻撃者またはPickupの基準座標に加算する位置オフセット。`{ x = 0, y = 0 }` が既定値です。 |
| `duration` | number | `pickup`、`sound`、`blur_in`、`blur_out`、`concentration_lines`、`screen_shake`、`time_scale` | Pickupの表示時間、サウンドの再生時間、または効果の長さ。秒単位。 |
| `color` | table | `screen_flash` | 発光色。`{ r = 255, g = 255, b = 255, a = 255 }`形式。省略時は白です。0から1の値も指定できます。 |
| `fadeIn` | number | `screen_flash` | 発光へ徐々に変化する時間。秒単位。省略時は `0` 秒です。 |
| `hold` | number | `screen_flash` | 最大発光を維持する時間。省略時は `duration` の値を使用します。 |
| `fadeOut` | number | `screen_flash` | 通常表示へ戻る時間。秒単位。省略時は `0` 秒です。 |
| `underEffects` | boolean | `screen_flash` | `true` の場合、Effekseerエフェクトをフラッシュより手前に描画します。省略時は `false` です。 |
| `strength` | number | `blur_in`、`blur_out`、`concentration_lines`、`screen_shake` | ぼかし、集中線、振動の強さ。ぼかしと集中線の既定値は `2.5`、指定可能範囲は `0` から `8` です。画面振動はピクセル単位で指定します。 |
| `frequency` | number | `screen_shake` | 1秒あたりの振動回数。既定値は `30` です。 |
| `center` | Vector2D | `blur_in`、`blur_out`、`concentration_lines` | フォーカスの中心座標。画面座標のピクセル単位。省略時は画面中央です。 |
| `focusRadius` | number | `blur_in`、`blur_out`、`concentration_lines` | 中心からこの距離までは鮮明または集中線なしに保ちます。ピクセル単位。既定値は `180` です。 |
| `scale` | number | `time_scale` | 時間倍率。既定値は `0.5` です。 |

`effect` アクションの `EfekseerEmitter2D` は、常に攻撃者のグローバル座標で
再生されます。`offset` を指定した場合は、その値を攻撃者の座標へ加算します。

`sound` アクションは指定したサウンドを再生します。`duration` を指定すると、
再生開始からその秒数が経った時点で 0.2 秒かけてフェードアウトして停止します。
`duration` を省略した（または `0` の）場合は、サウンドを最後まで再生します。

```lua
{
    time = 0.5,
    type = "sound",
    sound = "sound/one_shot/select_2.oga",
    volume = 0.8,
    duration = 1.5
}
```

演出の開始前には、タイムラインで使うアセットがあらかじめ読み込まれます
（`BattleAttack::PreloadAssets`）。対象は `effect` のEffekseerエフェクト、
`pickup` のテクスチャ、`sound` のサウンド（こちらは `SoundPlayer` の生成まで）です。
エフェクトと `Pickup` のインスタンスは実行時に生成します。生成順が描画順に
なるため、開始前に生成すると、後から生成されるフラッシュ等より下に隠れて
しまうためです。

`screen_flash` アクションは画面全体を指定色の矩形で覆います。`underEffects = true` を指定すると、
攻撃演出中のEffekseerエフェクトをフラッシュより前面に表示できます。

```lua
{
    time = 0.8,
    type = "screen_flash",
    color = { r = 255, g = 255, b = 255, a = 255 },
    fadeIn = 0.1,
    hold = 0.2,
    fadeOut = 0.3,
    underEffects = true
}
```

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

## 演出終了後の処理（finally）

アクションに `finally = true` を付けると、そのアクションは通常のタイムライン
（演出開始からの `time`）ではなく、演出終了後の処理として実行されます。
この場合の `time` は、演出終了（`attack.duration` 到達）から実行するまでの
秒数です。`time` が 0 未満の場合は 0 として扱われます。

```lua
attack = {
    duration = 1.0,
    actions = {
        { time = 0.6, type = "damage", target = "target", amount = 30 },
        -- 演出終了の瞬間に待機アニメーションへ戻す
        { time = 0.0, finally = true, type = "animation", target = "actor", animation = 0 },
        -- 演出終了の0.5秒後に初期位置へ戻す
        { time = 0.5, finally = true, type = "position", target = "actor", reset = true }
    }
}
```

動作は次の通りです。

- 演出終了の時点で、通常アクション側のエフェクトや画面効果は従来通り停止します。その後に `finally` アクションが実行されるため、`finally` 側で開始したエフェクト等が終了処理で止められることはありません。
- `BattleAttack` の完了（`Completed` シグナルと `IsFinished()`）は、すべての `finally` アクションが実行された後になります。`finally` アクションが1つも無い場合は従来通り `duration` で完了します。
- 演出全体の終了時刻は、最も遅い `finally` アクションの実行時刻（`duration + time`）で決まります。`finally` アクション自体に再生時間がある場合（`effect` や `pickup` など）は、その分を見込んで後続の `finally` アクションの `time` を遅らせてください。最後の `finally` アクションが実行された時点で、残っているエフェクト等は停止します。
- `effect` アクションの `triggerTime` / `triggers` は、`finally` の場合も実行時刻からの相対秒として扱われます。

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
