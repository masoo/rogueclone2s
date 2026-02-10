# 怪物用1幅 PUA フォントの作成

## Context

rogueclone2s の怪物表示に絵文字風のピクトグラムを使いたいが、標準の絵文字は `wcwidth()` が2を返すため、ターミナルで2セル幅になりゲームグリッドが崩れる。全ターミナルで1セル幅になる Private Use Area (PUA: U+E000〜) にグリフを配置する。

フォント作成には **Glyphr Studio v2**（ブラウザベース、インストール不要）を使用する。

## 作業手順

### 手順 1: Glyphr Studio でプロジェクト作成

1. https://www.glyphrstudio.com/app を開く
2. プロジェクト名: `RogueClone2s Monsters`
3. 「Start a new font from scratch」を選択

### 手順 2: PUA 文字範囲の追加

1. **Settings** → **Project** タブを開く
2. Character Ranges セクションで **カスタム範囲を追加**
3. 範囲: `U+E000` 〜 `U+E019`（26文字分）
4. 範囲名: `Monster Pictograms`

### 手順 3: フォントメトリクスの設定

**Settings** → **Font** タブで Key Metrics を設定:

| 項目 | 値 | 説明 |
|------|-----|------|
| UPM (Units Per Em) | 1000 | 標準値 |
| Ascent | 800 | ベースラインより上 |
| Descent | 200 | ベースラインより下 |

**重要**: 全グリフの advance width を統一する（モノスペース）。
半幅（1セル分）にするため、advance width を **全角の半分**に設定。
一般的なモノスペースフォントの全角幅が 1000 なら、半幅は **500**。

### 手順 4: グリフのデザイン

**Characters** ページで U+E000〜U+E019 の各コードポイントにグリフを描画。

グリフ作成の選択肢:
- **直接描画**: Glyphr Studio のベクター編集ツールで描く
- **SVG インポート**: Inkscape 等で作った SVG をドラッグ＆ドロップまたはコピーペースト

デザインのガイドライン:
- 描画領域: 幅 500 × 高さ 1000 ユニット（半幅モノスペース比率）
- シンプルなシルエット/アイコン（ターミナルの小さいフォントサイズで視認できること）
- モノクロ（単色）

参考デザイン元: [Google Noto Emoji](https://github.com/googlefonts/noto-emoji)（Apache 2.0 ライセンス）の SVG を縮小・簡略化して利用可能。

### 怪物とグリフのマッピング

| PUA | 怪物 | デザイン元の絵文字 | グリフイメージ |
|-----|------|--------------------|---------------|
| U+E000 | Aquatar | 💧 | 水滴のシルエット |
| U+E001 | Bat | 🦇 | コウモリの翼 |
| U+E002 | Centaur | 🐴 | 馬の頭 |
| U+E003 | Dragon | 🐉 | 龍のシルエット |
| U+E004 | Emu | 🐦 | 鳥のシルエット |
| U+E005 | Flytrap | 🌿 | 植物の葉 |
| U+E006 | Griffin | 🦅 | 鷲の頭 |
| U+E007 | Hobgoblin | 👹 | 鬼の顔 |
| U+E008 | Ice Monster | 🧊 | 氷の結晶 |
| U+E009 | Jabberwock | 🐲 | 竜の顔 |
| U+E00A | Kestrel | 🦉 | フクロウの顔 |
| U+E00B | Leprechaun | 🍀 | 四葉のクローバー |
| U+E00C | Medusa | 🐍 | 蛇の頭（髪） |
| U+E00D | Nymph | 🧚 | 妖精の羽 |
| U+E00E | Orc | 👺 | 天狗の顔 |
| U+E00F | Phantom | 👻 | おばけ |
| U+E010 | Quagga | 🦓 | シマウマの頭 |
| U+E011 | Rattlesnake | 🐍 | とぐろを巻く蛇 |
| U+E012 | Snake | 🐍 | 蛇（細い） |
| U+E013 | Troll | 🧌 | トロルの顔 |
| U+E014 | Unicorn | 🦄 | ユニコーンの角 |
| U+E015 | Vampire | 🧛 | 吸血鬼の牙 |
| U+E016 | Wraith | 💀 | 頭蓋骨 |
| U+E017 | Xeroc | 🫥 | 不定形のかたまり |
| U+E018 | Yeti | 🦍 | ゴリラ/雪男 |
| U+E019 | Zombie | 🧟 | ゾンビの手 |

### 手順 5: モノスペース化

1. **Global Actions** ページを開く
2. 「Set all advance widths to...」で **500** を指定（半幅統一）
3. これにより全グリフが同じ advance width になり、モノスペースフォントとして認識される

### 手順 6: エクスポート

1. **Import/Export** ページを開く
2. **OTF** または **TTF** 形式でエクスポート
3. ファイル名: `RogueClone2sMonsters-Regular.ttf`

### 手順 7: フォントのインストール

```bash
# Linux
mkdir -p ~/.local/share/fonts
cp RogueClone2sMonsters-Regular.ttf ~/.local/share/fonts/
fc-cache -fv

# Windows (WSL2 からホスト側)
cp RogueClone2sMonsters-Regular.ttf /mnt/c/Users/<ユーザー名>/AppData/Local/Microsoft/Windows/Fonts/
```

### 手順 8: ターミナルのフォールバック設定

**Windows Terminal** (`settings.json`):
```json
{
    "profiles": {
        "defaults": {
            "font": {
                "face": "メインフォント名",
                "fallback": ["RogueClone2s Monsters"]
            }
        }
    }
}
```

**Linux fontconfig** (`~/.config/fontconfig/fonts.conf`):
```xml
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "fonts.dtd">
<fontconfig>
  <alias>
    <family>メインフォント名</family>
    <prefer>
      <family>メインフォント名</family>
      <family>RogueClone2s Monsters</family>
    </prefer>
  </alias>
</fontconfig>
```

## 検証方法

```bash
# 1. フォントが認識されているか
fc-list | grep RogueClone2s

# 2. PUA 文字が1セル幅で表示されるか
# U+E000 = UTF-8: EE 80 80
# U+E019 = UTF-8: EE 80 99
printf 'A\xEE\x80\x80B'  # A[aquatar]B が3セル幅で表示されるはず

# 3. wcwidth の確認（既存コードで）
# src/wcwidth.c の wcwidth() は PUA に対して 1 を返す（確認済み）
```

## 成果物

- `RogueClone2sMonsters-Regular.ttf` — 26体の怪物ピクトグラムを PUA に持つフォント
- Glyphr Studio のプロジェクトファイル（`.gs2` 形式で保存しておくと後から編集可能）

## 注意事項

- **スコープ**: フォント作成のみ。ゲームコードの PUA 対応は別タスク
- **ライセンス**: Noto Emoji の SVG を参考にする場合は Apache 2.0 ライセンス表記が必要
- **ターミナル互換性**: PUA は `wcwidth()` が1を返すため、全ターミナルで1セル幅
- **Glyphr Studio のプロジェクト保存**: ブラウザベースのため、作業中は頻繁にプロジェクトファイル（`.gs2`）をダウンロード保存すること
