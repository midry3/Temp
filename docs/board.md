# 将棋ライクな盤面

盤面は `Game/Board/BoardBase` が管理し、`Assets/board/default.lua` から
盤面サイズ、タイル、ユニット配置を読み込みます。

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

## 操作

味方ユニットをクリックして選択し、移動先のタイルをクリックします。
選択中は移動可能なタイルが白く点滅します。右クリックすると選択をキャンセルできます。
味方ユニットが全員1回移動するとターンが終了し、敵が`y`方向へ1マス移動します。
敵ユニットが通行不可タイルや他ユニットに阻まれた場合はその場に留まります。

`BoardBase::EndTurn()` を呼ぶと、味方の未移動状態に関係なく敵を移動させて
ターンを進められます。
