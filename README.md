# IC Standard Guitar

インスタコード（InstaChord）を USB MIDI で Mac / Logic Pro に接続してリアルタイム演奏するために最適化された、オープンソースの Audio Unit (`.component`) および Standalone アプリケーションです（Apple Silicon / Intel Universal Binary 対応）。

Unreal Instruments の **Standard Guitar**（各自で取得したローカルの FLAC サンプル）を `sforzando` を介さず直接読み込み、インスタコード特有のレイテンシー（発音の遅れ）やキーオフ時の唐突な音切れを解消します。

> **免責・非公式表記**: 本ソフトウェアは個人開発による非公式プラグインであり、インスタコード（InstaChord / 株式会社KANTAN）および Unreal Instruments とは一切関係ありません。また、Standard Guitar の音声サンプルデータ（`.flac` 等）は本リポジトリおよび配布バイナリに一切同梱していません。

## 主な機能

- **自動プリ・アタック除去 (`Attack Tightness`)**
  - 各サンプル先頭に含まれるピッキング前の無音・立ち上がり遅延を解析してトリミングし、短時間フェードインによりクリックノイズなしで即座に発音します。
- **キーオフ・ダンパー物理 (`Key-Off Release` / `Damper Softness`)**
  - `Note Off` 時に一律で音を切る代わりに、指数減衰＋ボイスごとの 1次ローパス・ダンパーフィルターにより、弦を手で押さえたときのように高域から丸く自然に減衰させます。
- **和音リリースノイズ間引き (`Release Noise`)**
  - コード解放時に全弦分のリリースノイズが同時に鳴るのを防ぐため、80ms のクールダウンで和音あたり最大 1 回のみ控えめに鳴らします。
- **自動ストローク方向検出 (`Auto Strum`)**
  - 85ms 以内の連続ノートのピッチ進行からダウン／アップストロークを自動判定し、`Sus_Down` / `Sus_Up`（または `Mute_Down` / `Mute_Up`）を切り替えます。
- **インスタコード CC#80 ルートボタン連動ミュート (`IC CC#80 Auto`)**
  - インスタコードのルートボタン押下／解放で送出される `CC#80` に連動し、実音（`Sus_Down` / `Sus_Up`）とミュート音（`Finger Mute` または `Palm Mute`）をリアルタイムに切り替えます（極性反転ボタン・ヘッダーステータス表示付き）。
- **同音クロスフェード**
  - 同じ音程が連打された際、既存のボイスを 8ms でクロスフェード・アウトさせ、途切れや不自然なクリックを防ぎます。

## バージョニング規則

- **現在バージョン**: `v1.1.0`
- **機能追加（MINOR）**: `+0.1.0` ずつカウントアップ（例: `1.0.0` → `1.1.0` → `1.2.0`）
- **不具合修正のみ（PATCH）**: `+0.0.1` ずつカウントアップ（例: `1.1.0` → `1.1.1`）

---

## インストール手順（配布 ZIP から使う場合）

1. **Standard Guitar 本体の準備**
   - Unreal Instruments 公式から `UI_Standard_Guitar` をダウンロードし、解凍したフォルダを `~/Music/Logic/UI_Standard_Guitar`（または `~/Music/UI_Standard_Guitar` / `~/Downloads/UI_Standard_Guitar` 等）に配置します。
2. **プラグインの配置**
   - GitHub Releases から `IC-Standard-Guitar-v1.1.0-macOS-Universal.zip` をダウンロードして解凍します。
   - `IC Standard Guitar.component` を以下のフォルダへコピーします：
     - `~/Library/Audio/Plug-Ins/Components/`（または `/Library/Audio/Plug-Ins/Components/`）
3. **macOS Gatekeeper の隔離属性を解除（初回のみ）**
   - 本プラグインは Apple Developer ID による公証を行っていないため、そのまま起動すると「壊れているため開けません」「開発元を検証できません」と表示されます。配置後にターミナルで以下のコマンドを実行してください：
   ```bash
   xattr -cr ~/Library/Audio/Plug-Ins/Components/"IC Standard Guitar.component"
   ```
   - Standalone アプリ（`IC Standard Guitar.app`）を使う場合は同様に以下を実行してください：
   ```bash
   xattr -cr "IC Standard Guitar.app"
   ```
4. **Logic Pro での選択とサンプル指定**
   - Logic Pro を再起動し、ソフトウェア音源トラックの音源スロットから **AU 音源 ＞ InstaChordTools ＞ IC Standard Guitar** を選択します。
   - もしサンプルが自動検出されない場合は、プラグイン画面右上の **「フォルダ変更...」** から `UI_Standard_Guitar/Samples`（または `UI_Standard_Guitar`）フォルダを選択してください。一度設定すると `~/Library/Application Support/ICStandardGuitar/settings.xml` に保存され、次回以降も自動的に読み込まれます。

---

## ソースからのビルド手順（開発者向け）

### 必要環境

- macOS 12.0+（Universal Binary: `arm64` + `x86_64`）
- CMake 3.22+ / Xcode Command Line Tools
- ローカル JUCE 8（リポジトリ内の `./JUCE`、`$HOME/JUCE`、または `-DJUCE_DIR=/path/to/JUCE` で指定）

### ビルド・パッケージング

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR=/path/to/JUCE
cmake --build build --config Release -j8
```

ビルドが成功すると、`IC Standard Guitar.component` が自動的に `~/Library/Audio/Plug-Ins/Components/` にインストールされます（`COPY_PLUGIN_AFTER_BUILD TRUE`）。

配布用 ZIP アーカイブを作成する場合：

```bash
./Scripts/package-release.sh
```

`dist/IC-Standard-Guitar-v1.1.0-macOS-Universal.zip` が生成されます。

### テスト・検証

```bash
ctest --test-dir build --output-on-failure
auval -v aumu Icsg Icht
```

## ディレクトリ構成

- `CMakeLists.txt` — JUCE Universal Binary ビルド・テスト構成
- `Source/SampleBank.{h,cpp}` — FLAC 非同期ローダー、音名/RR解析、オンセット検出、パス設定保存
- `Source/GuitarSynthEngine.{h,cpp}` — 24音ポリフォニックエンジン、ダンパー減衰、Tone EQ、空間リバーブ
- `Source/PluginProcessor.{h,cpp}` — AU / Standalone プロセッサ、APVTS パラメータ、状態保存、試奏トリガー
- `Source/PluginEditor.{h,cpp}` — 日本語対応 UI、ステータス表示、ワンクリック試奏ボタン
- `Tests/EngineVerificationTest.cpp` — C++ エンジン検証テスト
- `Scripts/package-release.sh` — 配布用 ZIP パッケージ生成スクリプト
- `LICENSE` — GNU Affero General Public License v3.0 (AGPL-3.0)

## ライセンス

本プロジェクトは **GNU Affero General Public License v3.0 (AGPL-3.0)** のもとで公開されています（詳細は [LICENSE](LICENSE) を参照）。
