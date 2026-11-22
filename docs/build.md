# ビルド確認手順 (Build Instructions)

本プロジェクト (`OnBoard`) のビルド確認手順です。
AI エージェントや開発者がコード修正後の動作・コンパイル確認を行う際は、以下の手順に従ってください。

## 1. ビルドコマンド

プロジェクトのルートディレクトリ (`c:\Users\midry\projects\Novot-Engine\OnBoard`) で、PowerShell (pwsh) から次のコマンドを実行します。

### Debug ビルド (標準確認用)
```powershell
msbuild OnBoard.vcxproj /p:Configuration=Debug /p:Platform=x64 -m
```

### Release ビルド
```powershell
msbuild OnBoard.vcxproj /p:Configuration=Release /p:Platform=x64 -m
```

### クリーン＆リビルド
```powershell
msbuild OnBoard.vcxproj /t:Rebuild /p:Configuration=Debug /p:Platform=x64 -m
```

## 2. ビルド仕様・特徴

- **プラットフォーム**: `x64`
- **構成 (Configuration)**: `Debug` または `Release`
- **依存関係**: `OnBoard.vcxproj` は `../Novot/Novot.vcxproj` を参照しており、MSBuild 実行時にエンジン側のライブラリ (`Novot.lib`) も自動でビルドされます。
- **並列ビルド**: `-m` (マルチプロセッサビルド) を指定することで高速化されます。

## 3. ビルド結果の判定

- **成功の目安**: 出力の末尾に `0 エラー` (`0 Error(s)`) と表示されればビルド成功です。
- **既知の警告 (無視可能)**:
  - `EffekseerRendererCommon_vs2022_x64_d.lib` や `EffekseerForDXLib` 等に関する `warning LNK4099: PDB ... が見つかりません` はサードパーティ製ライブラリのデバッグシンボルに関するものであり、問題ありません。

## 4. エージェント向け実行時の注意点

- **所要時間**: 初回ビルドや中間生成物がない場合、Novot エンジンのコンパイルを含め数分（10分程度）かかる場合があります。差分ビルド時は数十秒程度で終了します。
- **バックグラウンド実行**: 時間がかかるコマンドはバックグラウンドタスクとして非同期実行し、完了通知を待機してください。
- **コーディング規約の遵守**: コンパイルが通るだけでなく、[`Rule.md`](Rule.md) の規約（`>`, `>=` の不使用、`not` の使用など）を満たしていることを確認してください。
