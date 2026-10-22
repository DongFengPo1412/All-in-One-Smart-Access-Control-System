# Three-Mode Access Control Prototype — Setup and operation

[简体中文](setup.zh-CN.md) · **English** · [日本語](setup.ja.md) · [Overview](../README_EN.md)

<a id="hardware"></a>

## Actual interfaces

| Device | Source configuration |
| :--- | :--- |
| MCU | STM32F103C8T6, HSE×9/expected 72 MHz; linker 64 KiB Flash/20 KiB RAM. |
| AS608 | USART1 PA9 TX/PA10 RX,57600 8N1. |
| K210 | UART1 IO9 TX/IO10 RX to STM32 USART2 PA3 RX/PA2 TX,115200 8N1. |
| Debug | USART3 PB10 TX/PB11 RX,115200; printf uses this port. |
| RC522 | MISO PA5, RST PA6, SCK PA7, CS/SDA PB0, MOSI PB1, software SPI. |
| OLED | PB6 SCL/PB7 SDA, software I2C-like signaling. |
| Servo | PB9 TIM4_CH4. |
| Four keys | PB12–15 pull-ups, active low. |
| K210 enrollment | IO16 falling-edge input; QVGA RGB565 camera, LCD, SD. |

Check actual module power/levels and servo current. Source does not prove a rated supply or star grounding. Do not follow reversed USART1/3 assignments in old diagrams.

<a id="structure"></a>

## Structure and toolchains

```text
.
├── STM32_Project/   # CubeIDE, Core, HAL/CMSIS, linker, ioc
├── K210_Project/    # boot.py, three .smodel files, device warning
├── Face_recognitio_images/   # historical images
├── Instructions.pdf
└── docs/           # guides and photos
```

STM32 needs CubeIDE/ARM tools and a programmer; K210 needs firmware compatible with the older MaixPy KPU API. Desktop pip packages are not a device runtime. The [model warning](../K210_Project/redme.md) explicitly requires machine-code-matched encrypted models.

<a id="deployment"></a>

## Deploy separately

1. Import the project identified by `STM32_Project/.project`; preserve handwritten files rather than regenerating from ioc.
2. Inspect main's HSE/peripherals, toolchain printf redirection, USART1 reception, and 0x08009000 page. Check the map before programming; this audit does not prove a successful full link.
3. Build/program STM32 with the actual programmer. No guaranteed cross-machine one-command build is supplied.
4. Obtain K210 machine-matched models at the SD root:`FaceDetection.smodel`,`FaceLandmarkDetection.smodel`,`FeatureExtraction.smodel`.
5. Prepare `/sd/data.txt`, initially empty if needed. Source opens it for reading without tolerant creation. Place `boot.py` and verify LCD/camera/IO9/10/16.
6. Check boards/UART separately before connecting the servo mechanism. Boolean events are not authenticated authorization frames.

Existing local `K210_Project/data.txt` is ignored and absent from clones. Do not commit face templates or identity data.

<a id="operation"></a>

## Operation and historical photos

Startup enters access mode. Key1 opens/cycles menus, Key2 confirms; Key3/4 generally return to menu/home, with page-specific differences in main. Card/fingerprint enrollment slots are 0–3. Face enrollment uses the K210 board button; STM32 does not provide working remote face enrollment/clear.

| Historical card | Historical fingerprint | Historical face |
| :---: | :---: | :---: |
| ![Card](images/demo_live_rfid_card_test.jpg) | ![Fingerprint](images/demo_live_fingerprint_test.jpg) | ![Face](images/demo_live_k210_face_test.jpg) |

These record the course prototype, not this audit's testing or security approval. [Instructions PDF](../Instructions.pdf) is secondary; source takes precedence on conflicting pins/parameters.

<a id="checks"></a>

## Checks and troubleshooting

Only parsing/ARM syntax checks ran, without serial access or servo motion.

| Problem | Inspect |
| :--- | :--- |
| Stuck loading | AS608 power, USART1, handshake, IDLE/RX_len; no overall startup timeout. |
| K210 boot failure | Machine code/models, Maix version, SD/data.txt, camera/LCD. |
| Face accepted but no motion | ASCII 1,115200,crossed TX/RX,face_flag,blocking host page. |
| Lost/invalid enrollment | Three storage locations; Flash map, SD write, module response. |
| Different servo angle | CCR 50/175, actual clock, mechanism/power; not assumed 0°/90°. |
| Peripherals disappear after generation | The ioc does not describe handwritten code; restore and reconcile configuration. |
