# Three-Mode Access Control Prototype — 技術ノート

[简体中文](technical-notes.zh-CN.md) · [English](technical-notes.en.md) · **日本語** · [概要](../README_JA.md)

<a id="source-map"></a>

## ソースと版の根拠

| 部分 | ソース・役割 |
| :--- | :--- |
| 主処理 | [main.c](../STM32_Project/Core/Src/main.c)：初期化、6ページ、3経路、登録削除。 |
| UART | [usart.c](../STM32_Project/Core/Src/usart.c)、[割込み](../STM32_Project/Core/Src/stm32f1xx_it.c)：指紋受信・顔フラグ。 |
| 機器 | [AS608.c](../STM32_Project/Core/Src/AS608.c)、[RC522.c](../STM32_Project/Core/Src/RC522.c)：パケット・レジスタ。 |
| UI・実行 | [OLED.c](../STM32_Project/Core/Src/OLED.c)、[key.c](../STM32_Project/Core/Src/key.c)、[Servo.c](../STM32_Project/Core/Src/Servo.c)、[tim.c](../STM32_Project/Core/Src/tim.c)。 |
| 保存 | [myflash.c](../STM32_Project/Core/Src/myflash.c)、[定数](../STM32_Project/Core/Inc/myflash.h)。 |
| 視覚 | [boot.py](../K210_Project/boot.py)、[モデル制約](../K210_Project/redme.md)。 |
| 工程 | [ioc](../STM32_Project/SmartAccessControlSystem.ioc)、[リンク](../STM32_Project/STM32F103C8TX_FLASH.ld)、`.cproject`。 |

授業 `7Project` を基に保守修正したため、現在は保存版とバイト一致しません。写真とモデルは保持し、異なる過去版の機能は混在させていません。

<a id="authentication"></a>

## 3つの判定

**カード：** 主ループは`PcdRequest/PcdAnticoll`とUID比較です。レベル1衝突防止で4バイトとXOR BCCを確認します。選択・CRC・セクタ認証・読書き関数もありますが、解錠経路で暗号認証や完全な複数カード二分木探索を実行する根拠にはなりません。

**指紋：** MCUはEF01・アドレス・種別・長さ・命令・加算チェックサムを構成します。特徴・検索はAS608内部で、想像上の特徴点グラフ式をMCU独自アルゴリズムとはしません。2回採取でbuffer1/2、Match、RegModel、StoreCharを実行し、UIは0–3です。0から300項目を検索し、成功かつ`mathscore>100`で解錠します。百分率ではありません。

**顔：** KPU YOLO2、ランドマーク、5点アフィン、特徴ネット、`face_encode/face_compare`です。コメントは196次元としますが構造・学習・出力形状の検証はなく、独自学習や特定の正規化コサイン式は断定しません。SDKスコア`>80`はコサイン0.72や認識率80%ではありません。ライブネス・写真攻撃・FAR/FRRは未評価です。

K210の現地登録・読込は名前に対応する10件に制限し、消去関数のグローバル更新を修正しました。遠隔A/B/C入力は無効のままです。顔イベントはASCII `1`とベストエフォートの確認送信であり、ID・連番・リプレイ防止はありません。製品用の認証済み錠ではありません。

<a id="storage-actuation"></a>

## 保存とサーボ

UIDは従来の `0x08009000/04/08/0C`、各4バイトを維持します。プログラム領域を36 KiBに制限してUIDページを予約し、超過はリンクエラーにします。物理Flashは64 KiBで、ページ以降はプログラムに割り当てません。書込はUIDページ内の整列範囲に制限し、HAL失敗をUIへ返します。トランザクションと停電復旧はなく、登録を保持する際は書込器の全消去を避ける必要があります。



タイマ72 MHz、PSC719、ARR1999の場合：

```math
T=\frac{(719+1)(1999+1)}{72\times10^6}=0.020\,s,\qquad t_{high}=CCR\times10\,\mu s
```

CCR50/175は0.5/1.75 msで、旧CCR500/1500や校正済み0°/90°ではありません。`ControlServo`は3 s待って戻り、扉・錠・角度のフィードバックはありません。

<a id="state"></a>

## 主ループと設計上の制約

page0はface_flag、キー、カード、カードなしなら指紋を調べます。page1は4管理項目、page2–5はカード・指紋の追加削除です。キーは20 ms待って解放までブロックし、応答・失敗表示も待ちます。

指紋ACKに対し長さ、アドレス、加算検査符号を有界に検査します。受信長は16ビットとし、ISRは独立スナップショットを公開、IDLEを明示的に有効化しました。USART2は非ブロッキングの確認送信と1回の受信再設定を使用します。AS608起動は最大5回で、未接続時は指紋操作を無効にしてカード・顔経路を維持します。

OLEDはPB6/7ソフト時序、書込0x78、ACK無検査で、ハードI2Cではありません。RC522もGPIO SPIです。2チップへの分担は電気的絶縁や安全境界の証明ではありません。

<a id="implementation-status"></a>

## 検証と未完成部分

ARM GCC 13.3でコンパイルとリンクに成功しました。text 19,952 B、data 128 B、bss 4,760 Bです。ClangでWebAssemblyにした独立ACK解析テスト8件がNodeで通過しました。K210は構文検査のみです。書込・周辺実測・新たな認識性能評価は行っていません。

`.ioc` は基本RCC/NVICのみで、手書き72 MHz・周辺設定と一致しません。再生成は検証済みの復元手順ではなく、ビルド成功だけで実機UARTタイミングは確認できません。

授業 `7Project` を基に保守修正したため、現在は保存版とバイト一致しません。写真とモデルは保持し、異なる過去版の機能は混在させていません。
