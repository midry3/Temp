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
- `Place character` は外部データのキャラクター一覧から選び、味方または敵の初期位置を置きます。味方はキャラクター種別ごとに1体、敵は同じ種別を複数配置できます。
- マウス右ボタンをドラッグするとカメラが移動し、ホイールでカーソル位置を中心に拡大・縮小します。
- `Save` は変更を Lua 盤面ファイルに書き出し、`Load` は指定したファイルを読み直します。

キャラクターの `displayName` と `icon`、マップ上のタイル画像・通行可否、オブジェクト、敵味方の配置は Lua ファイルに保存されます。キャラクターのクラス対応は `BoardCharacterType` と
`CreateBattleCharacter` に集約し、新しいクラスを追加するときは enum とファクトリーの対応を追加します。

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

## 操作

味方ユニットをクリックして選択し、移動先のタイルをクリックします。
選択中は移動可能なタイルが白く点滅します。右クリックすると選択をキャンセルできます。
味方ユニットが全員1回移動するとターンが終了し、敵が`y`方向へ1マス移動します。
敵ユニットが通行不可タイルや他ユニットに阻まれた場合はその場に留まります。

`BoardBase::EndTurn()` を呼ぶと、味方の未移動状態に関係なく敵を移動させて
ターンを進められます。
