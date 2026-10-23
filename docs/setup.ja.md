# Three-Mode Access Control Prototype — 導入と操作

[简体中文](setup.zh-CN.md) · [English](setup.en.md) · **日本語** · [概要](../README_JA.md)

<a id="hardware"></a>

## 実際の接続

| 部品 | ソース設定 |
| :--- | :--- |
| MCU | STM32F103C8T6、HSE×9／想定72 MHz。物理Flash64 KiB、リンクするコード36 KiB、RAM20 KiB。 |
| AS608 | USART1 PA9 TX／PA10 RX、57600 8N1。 |
| K210 | UART1 IO9 TX／IO10 RX→STM32 USART2 PA3 RX／PA2 TX、115200 8N1。 |
| デバッグ | USART3 PB10 TX／PB11 RX、115200、printf出力。 |
| RC522 | MISO PA5、RST PA6、SCK PA7、CS/SDA PB0、MOSI PB1、ソフトSPI。 |
| OLED | PB6 SCL／PB7 SDA、ソフトI2C式。 |
| サーボ | PB9 TIM4_CH4。 |
| 4キー | PB12–15、プルアップ、Low有効。 |
| K210登録 | IO16下降入力、QVGA RGB565、LCDとSD。 |

実物の電源・レベル・サーボ電流を確認します。定格電源や一点接地はコードから証明できません。旧図のUSART1/3逆転を接続根拠にしません。

<a id="structure"></a>

## 構成とツール

```text
.
├── STM32_Project/   # CubeIDE、Core、HAL/CMSIS、リンク、ioc
├── K210_Project/    # boot.py、3つの.smodel、機器制約
├── Face_recognitio_images/   # 過去の画像
├── Instructions.pdf
└── docs/           # ガイドと写真
```

STM32はCubeIDE・ARMツール・書込み機器、K210は旧MaixPy KPU API互換ファームウェアが必要です。PCのpipで代替しません。[モデル注意](../K210_Project/redme.md)は個別機器コードに対応する暗号モデルを要求します。

<a id="deployment"></a>

## 別々の導入

1. `STM32_Project/.project`の工程を取り込み、手書きファイルを保ち、iocから上書き生成しません。
2. HSE・周辺・printf・USART1・UIDページとmapを確認します。ARM GCC全体リンクは検証済みですが、CubeIDE再生成や実機書込みの成功を保証しません。
3. 実際の書込み機器でSTM32をビルド・導入します。環境を問わない成功保証付き1コマンドはありません。
4. K210機器コード対応モデルをSDルートの`FaceDetection.smodel`、`FaceLandmarkDetection.smodel`、`FeatureExtraction.smodel`へ置きます。
5. `/sd/data.txt`を用意します。初回は空でもよいですが、コードは読込で開き自動作成しません。`boot.py`とLCD・カメラ・IO9/10/16を確認します。
6. 両機器・UARTを個別確認し、サーボ模擬機構へ接続します。真偽イベントを安全な認証通信とは扱いません。

本機の`K210_Project/data.txt`はGit対象外です。顔テンプレートや個人情報をコミットしません。

<a id="operation"></a>

## 操作と過去の写真

既定は解錠ページです。Key1でメニュー・項目巡回、Key2で確認、Key3/4で通常メニュー／ホームへ戻りますが、ページ差はmainを確認します。カード・指紋は0–3。顔登録はK210基板キーで、STM32からの遠隔登録・削除は動作経路がありません。

| 過去カード | 過去指紋 | 過去顔 |
| :---: | :---: | :---: |
| ![カード](images/demo_live_rfid_card_test.jpg) | ![指紋](images/demo_live_fingerprint_test.jpg) | ![顔](images/demo_live_k210_face_test.jpg) |

授業試作の写真で、今回の試験・安全認証ではありません。[説明PDF](../Instructions.pdf)は補助資料で、接続・引数の相違はソースを優先します。

<a id="checks"></a>

## 検査と切り分け

ARM GCC 13.3でコンパイルとリンクに成功しました。text 19,952 B、data 128 B、bss 4,760 Bです。ClangでWebAssemblyにした独立ACK解析テスト8件がNodeで通過しました。K210は構文検査のみです。書込・周辺実測・新たな認識性能評価は行っていません。

| 現象 | 確認 |
| :--- | :--- |
| loadingから進まない | AS608電源、USART1、握手を確認。最大5回後に指紋をオフラインとし、カード・顔を維持します。 |
| K210起動失敗 | 機器コード・モデル、Maix、SD/data.txt、カメラ/LCD。 |
| 顔成功でも不動 | ASCII1、115200、TX/RX交差、face_flag、主機のブロック状態。 |
| 登録消失・異常 | 別々の保存先、Flash map、SD書込、モジュール応答。 |
| サーボ角度差 | CCR50/175、実クロック、機構・電源。0°/90°を仮定しません。 |
| 再生成後に周辺が消える | iocは現ソースを表さないため保存版から構成を照合します。 |
