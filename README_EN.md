# Three-Mode Access Control Prototype

A course access-control prototype combining STM32F103, K210 face processing, RFID, and a fingerprint module.

[简体中文](README.md) · **English** · [日本語](README_JA.md)

**Explore:** [Technical notes](docs/technical-notes.en.md) · [Setup and operation](docs/setup.en.md)

Card, fingerprint, and face matching are alternative access paths. STM32 manages menus, credentials, display, and a servo; K210 runs existing KPU detection/alignment/feature models, while AS608 performs fingerprint feature extraction and search internally. The engineering focus is peripheral protocols, state interaction, and device integration, rather than jointly trained modalities or mandatory three-factor authentication.

## System architecture

```mermaid
flowchart LR
    Card[RFID card] --> RC[MFRC522 software SPI]
    RC --> MCU[STM32F103C8T6]
    Finger[Fingerprint] --> AS[AS608 USART1]
    AS --> MCU
    Camera[Camera] --> KPU[K210 detection alignment matching]
    KPU -->|UART1 to USART2 ASCII 1| MCU
    MCU --> Servo[TIM4 servo]
    MCU --> OLED[OLED and four-key menu]
    MCU --> Flash[Four UID slots]
```

Branches are polled rather than all authentication running concurrently. Any accepted path can call the same unlock routine. K210 sends a one-byte event, not an authenticated identity/checksum/replay-protected protocol.

## Features and implementation

| Feature | Current source |
| :--- | :--- |
| Card access | Request/level-one anticollision, four-byte UID comparison against four registered slots. |
| Fingerprint access/management | USART1 at 57600; module capture, features, two captures/matching, template storage/search; unlock score strictly above 100. |
| Face access/enrollment | K210 YOLO2 API, five-point affine alignment, encoded features/library comparison; score strictly above 80 sends `1`; local button saves to SD. |
| Interaction/storage | OLED pages/four keys; UIDs in STM32 Flash, face features on SD, fingerprint templates in the module. |
| Lock demonstration | Two TIM4_CH4 PWM settings, blocking 3000 ms hold, then reset; servo mock mechanism. |

[Protocols, timing, and source](docs/technical-notes.en.md). No completed liveness detection, encrypted credential storage, two-factor policy, tamper alarm, or security certification is established.

## Demonstration and validation

![Course prototype](docs/images/demo_hardware_rig.png)

[Historical demonstration](https://www.bilibili.com/video/BV1qi3UzGEwH) and photos show the prototype, not measured sub-100 ms latency, FAR/FRR, or long-term reliability.

ARM GCC syntax checks passed for all Core C files; the K210 script parses statically. Linking, flashing, Maix model loading, and physical tests were not rerun. Main STM32 files match the local `7Project` copy; differing `6SmartGate` functionality was not merged into this system.

## Quick start

1. Follow the [source-derived pin table](docs/setup.en.md#hardware): fingerprint uses USART1; USART3 is debug.
2. Import `STM32_Project` into STM32CubeIDE, check toolchain/Flash usage/receive interrupts, then build and program.
3. K210 needs compatible Maix firmware, camera, LCD, SD, three models, and `/sd/data.txt`. The supplied warning requires encrypted models matching each board's machine code.
4. Follow [separate-device setup](docs/setup.en.md); the two applications are not one flash image.

The `.ioc` does not describe the current peripherals/72 MHz setup; regenerating it would not reproduce the handwritten source.

## Limits and next steps

Fingerprint responses, key release, and servo holding introduce blocking waits. Hard-real-time behavior and prompt face-event handling in every page are unverified. AS608 response parsing uses string search; reception/length handling needs hardware review.

The Flash region lacks linker reservation, transactions, and wear leveling. K210 remote enrollment/clearing is disconnected. Priorities are framed UART/timeouts, storage bounds, administrator authorization, and model reproduction before quantitative recognition/security evaluation. [Detailed limits](docs/technical-notes.en.md#implementation-status)

## Team and license

Historical documentation lists **Chaoran Liu, Shanlin Yuan, and Gaofeng Chen**. This describes the team system; exact personal ownership requires member records. Module-internal fingerprint algorithms, pretrained models, and inherited drivers are not individual original implementations.

Application license: [MIT](LICENSE). ST HAL/CMSIS, attributed peripheral/Flash code, Maix/KPU, and encrypted models retain their source terms. [Technical notes](docs/technical-notes.en.md) · [Setup](docs/setup.en.md)
