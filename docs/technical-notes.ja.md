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

main・AS608・RC522・usart・tim・Servo・myflashは本機`7Project`とバイト一致し、デスクトップの2アーカイブもbootが一致します。より新しい`6SmartGate`は異なるため、時刻だけで置換しません。

<a id="authentication"></a>

## 3つの判定

**カード：** 主ループは`PcdRequest/PcdAnticoll`とUID比較です。レベル1衝突防止で4バイトとXOR BCCを確認します。選択・CRC・セクタ認証・読書き関数もありますが、解錠経路で暗号認証や完全な複数カード二分木探索を実行する根拠にはなりません。

**指紋：** MCUはEF01・アドレス・種別・長さ・命令・加算チェックサムを構成します。特徴・検索はAS608内部で、想像上の特徴点グラフ式をMCU独自アルゴリズムとはしません。2回採取でbuffer1/2、Match、RegModel、StoreCharを実行し、UIは0–3です。0から300項目を検索し、成功かつ`mathscore>100`で解錠します。百分率ではありません。

**顔：** KPU YOLO2、ランドマーク、5点アフィン、特徴ネット、`face_encode/face_compare`です。コメントは196次元としますが構造・学習・出力形状の検証はなく、独自学習や特定の正規化コサイン式は断定しません。SDKスコア`>80`はコサイン0.72や認識率80%ではありません。ライブネス・写真攻撃・FAR/FRRは未評価です。

K210のASCII`1`にSTM32 USART2が`OK`を返しface_flagを設定します。送信側はOKを検査せず、ID・連番・CRC・リプレイ防止はありません。K210 UART読出しはコメント化され、A/B/C遠隔登録／消去に入力がありません。IO16キーの現地登録は実装済みです。消去はグローバルリスト更新に問題があり呼出しもコメント化されています。名前10項目に対して登録数制限がなく、添字超過の可能性があります。

<a id="storage-actuation"></a>

## 保存とサーボ

UID開始は`0x08009000/04/08/0C`、各4バイト・2半ワードです。スロット0に管理者特権はありません。ページの読出し・変更・書戻しはありますが、トランザクション、停電復旧、戻り値検証はありません。`STM32_FLASH_SIZE=0x10000`はバイトなのに境界判定で1024倍し、容量拡張ではなく境界不備です。

リンクは64 KiB全体をプログラムに許可し、UIDページを予約しません。mapで重複を確認します。指紋・UID・顔はモジュール／Flash／SDに分かれ、統一した安全ストアではありません。

タイマ72 MHz、PSC719、ARR1999の場合：

```math
T=\frac{(719+1)(1999+1)}{72\times10^6}=0.020\,s,\qquad t_{high}=CCR\times10\,\mu s
```

CCR50/175は0.5/1.75 msで、旧CCR500/1500や校正済み0°/90°ではありません。`ControlServo`は3 s待って戻り、扉・錠・角度のフィードバックはありません。

<a id="state"></a>

## 主ループと設計上の制約

page0はface_flag、キー、カード、カードなしなら指紋を調べます。page1は4管理項目、page2–5はカード・指紋の追加削除です。キーは20 ms待って解放までブロックし、応答・失敗表示も待ちます。

AS608起動ハンドシェイクは無期限再試行です。500バイト受信と8ビットRX_lenが一致せず、バイナリを`strstr`で探し完全な長さ・検査符号を確認しません。明示的なUSART1 IDLE有効化は見つからず、ISRもSRと`UART_IT_IDLE`を比較するためHAL定義・実機の確認が必要です。過去デモだけで現在の移植版の正しさは断定しません。

OLEDはPB6/7ソフト時序、書込0x78、ACK無検査で、ハードI2Cではありません。RC522もGPIO SPIです。2チップへの分担は電気的絶縁や安全境界の証明ではありません。

<a id="implementation-status"></a>

## 検証と未完成部分

ARM GNU13.3の`-fsyntax-only`でCore/Src全Cが通過しました。リンク・書込みは未実行です。K210はCPython構文解析のみで、Maix API・暗号モデル・SD・カメラ・LCDは実行していません。認識・時間・角度・電力の新測定もありません。

`.ioc`はRCC/NVICのみ、周辺0、8 MHzで、手書き72 MHz・周辺ソースと不一致です。再生成は検証済み復元手順ではありません。旧ツールのI/Oコードもあり、完全リンクの確認が必要です。

授業資料と写真は保持します。根拠のない産業品質・高安全・ライブネス・100 ms未満・警報・量子化学習・独自特徴点アルゴリズム・FAR/FRRは削除しました。通信・保存・時間制限・権限を先に改善し、モデル・環境再現と実測を進める方針です。ソース・写真・モデルは未変更です。
