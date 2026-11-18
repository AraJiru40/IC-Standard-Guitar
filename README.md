# IC Standard Guitar

インスタコード（InstaChord）を USB MIDI で Mac / Logic Pro に接続してリアルタイム演奏するために最適化された、個人利用向けの Audio Unit (`.component`) および Standalone アプリケーションです。

Unreal Instruments の **Standard Guitar**（ローカル配置済み FLAC サンプル）を `sforzando` を介さず直接読み込み、インスタコード特有のレイテンシー（発音の遅れ）やキーオフ時の唐突な音切れを解消します。

## 主な機能

- **自動プリ・アタック除去 (`Attack Tightness`)**
  - 各サンプル先頭に含まれるピッキング前の無音・立ち上がり遅延を解析してトリミングし、短時間フェードインによりクリックノイズなしで即座に発音します。
- **キーオフ・ダンパー物理 (`Key-Off Release` / `Damper Softness`)**
  - `Note Off` 時に一律で音を切る代わりに、指数減衰＋ボイスごとの 1次ローパス・ダンパーフィルターにより、弦を手で押さえたときのように高域から丸く自然に減衰させます。
- **和音リリースノイズ間引き (`Release Noise`)**
  - コード解放時に全弦分のリリースノイズが同時に鳴るのを防ぐため、80ms のクールダウンで和音あたり最大 1 回のみ控えめに鳴らします。
- **自動ストローク方向検出 (`Auto Strum`)**
  - 85ms 以内の連続ノートのピッチ進行からダウン／アップストロークを自動判定し、`Sus_Down` / `Sus_Up`（または `Mute_Down` / `Mute_Up`）を切り替えます。
- **同音クロスフェード**
  - 同じ音程が連打された際、既存のボイスを 8ms でクロスフェード・アウトさせ、途切れや不自然なクリックを防ぎます。

## 必要環境

- macOS (Apple Silicon / arm64)
- CMake 3.22+ / Xcode Command Line Tools
- ローカル JUCE（デフォルト設定: `/Users/daioshima/ClaudeCode/chord-pad-au/JUCE`、`CMakeLists.txt` の `JUCE_DIR` で変更可能）
- Unreal Instruments **UI_Standard_Guitar** の `Samples` フォルダ（デフォルト探索パス: `/Users/daioshima/Music/Logic/UI_Standard_Guitar/Samples`、UI の「フォルダ変更...」ボタンからも指定可能）

> **注意**: 著作権およびリポジトリ容量の観点から、音声サンプルファイル（`.flac` 等）およびビルド成果物は本リポジトリに含まれていません。

## ビルド手順

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j8
```

ビルドが成功すると、`IC Standard Guitar.component` が自動的に `~/Library/Audio/Plug-Ins/Components/` にインストールされます（`COPY_PLUGIN_AFTER_BUILD TRUE`）。

## テスト・検証

### エンジン自動検証テスト (`ICStandardGuitarTest`)

```bash
ctest --test-dir build --output-on-failure
```

サンプル読込数・立ち上がり遅延（2.5ms 未満）・キーオフ時のダンパー減衰特性・自動ストローク判定を検証します。

### Apple Audio Unit バリデーション

```bash
auval -v aumu Icsg Icht
```

## ディレクトリ構成

- `CMakeLists.txt` — JUCE ビルド・テスト構成
- `Source/SampleBank.{h,cpp}` — FLAC 非同期ローダー、音名/RR解析、オンセット検出
- `Source/GuitarSynthEngine.{h,cpp}` — 24音ポリフォニックエンジン、ダンパー減衰、Tone EQ、空間リバーブ
- `Source/PluginProcessor.{h,cpp}` — AU / Standalone プロセッサ、APVTS パラメータ、状態保存、試奏トリガー
- `Source/PluginEditor.{h,cpp}` — 日本語対応 UI、ステータス表示、ワンクリック試奏ボタン
- `Tests/EngineVerificationTest.cpp` — C++ エンジン検証テスト
