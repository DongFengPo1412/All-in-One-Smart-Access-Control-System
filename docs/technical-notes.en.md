# Three-Mode Access Control Prototype — Technical notes

[简体中文](technical-notes.zh-CN.md) · **English** · [日本語](technical-notes.ja.md) · [Overview](../README_EN.md)

<a id="source-map"></a>

## Source and version evidence

| Module | Source/responsibility |
| :--- | :--- |
| Main flow | [main.c](../STM32_Project/Core/Src/main.c): startup, six pages, three checks, enrollment/deletion. |
| UART | [usart.c](../STM32_Project/Core/Src/usart.c), [interrupts](../STM32_Project/Core/Src/stm32f1xx_it.c): fingerprint reception/face flag. |
| Devices | [AS608.c](../STM32_Project/Core/Src/AS608.c), [RC522.c](../STM32_Project/Core/Src/RC522.c): packets/register access. |
| UI/actuation | [OLED.c](../STM32_Project/Core/Src/OLED.c), [key.c](../STM32_Project/Core/Src/key.c), [Servo.c](../STM32_Project/Core/Src/Servo.c), [tim.c](../STM32_Project/Core/Src/tim.c). |
| Storage | [myflash.c](../STM32_Project/Core/Src/myflash.c), [constants](../STM32_Project/Core/Inc/myflash.h). |
| Vision | [boot.py](../K210_Project/boot.py), [model limitation](../K210_Project/redme.md). |
| Project | [ioc](../STM32_Project/SmartAccessControlSystem.ioc), [linker script](../STM32_Project/STM32F103C8TX_FLASH.ld),`.cproject`. |

main/AS608/RC522/usart/tim/Servo/myflash match local `7Project` byte-for-byte; both desktop archived boot files also match. Newer `6SmartGate` differs and is not substituted solely by timestamp.

<a id="authentication"></a>

## What the three checks do

**Card:** main calls `PcdRequest/PcdAnticoll` and compares UIDs. Level-one anticollision retrieves four bytes with XOR BCC. Selection/CRC/sector authentication/read-write routines exist in the driver, but the unlock path does not use them to establish cryptographic authentication or a complete multi-card binary-tree traversal.

**Fingerprint:** MCU forms EF01/address/type/length/command/additive-checksum packets. Features/search run inside AS608; imagined minutiae graph equations are not MCU algorithms. Enrollment captures buffers 1/2, Match, RegModel, StoreChar; UI slots are 0–3. Search starts at 0 over 300 entries; successful response and `mathscore>100` unlock. This is not a percentage.

**Face:** KPU YOLO2 detection, landmark network, five-point affine alignment, feature network,`face_encode/face_compare`. Comments say 196 dimensions, but architecture/training/output-shape evidence is absent; no original training or specific normalized cosine equation is claimed. SDK score `>80` is neither cosine 0.72 nor 80% accuracy. Liveness/photo resistance/FAR/FRR are not evaluated.

K210 sends ASCII `1`; STM32 USART2 replies `OK` and sets face_flag. Sender does not validate OK; identity, sequence, CRC, and replay protection are absent. K210 UART reading is commented out, leaving A/B/C remote enrollment/clear without input. Local IO16 enrollment exists. Clearing has local-list scope issues and is commented out. Ten names coexist with an unbounded registration list, risking indexing errors.

<a id="storage-actuation"></a>

## Persistence and servo

Four UID starts are `0x08009000/04/08/0C`, four bytes/two halfwords each. Slot 0 has no special administrator privilege. Page read-modify-write preserves other data but lacks transactions, power-failure recovery, and checked return codes. `STM32_FLASH_SIZE=0x10000` is bytes but the address guard multiplies it by 1024: a bounds defect, not extra capacity.

The linker still exposes all 64 KiB to program code without reserving the UID page. Inspect the map for overlap. Fingerprints, UIDs, and faces reside in separate module/Flash/SD storage, not one secure store.

For the configured 72 MHz timer clock, PSC 719, ARR 1999:

```math
T=\frac{(719+1)(1999+1)}{72\times10^6}=0.020\,s,\qquad t_{high}=CCR\times10\,\mu s
```

CCR 50/175 therefore produce 0.5/1.75 ms, not the old CCR 500/1500 values or calibrated 0°/90°. `ControlServo` waits 3 s then resets, with no door/bolt/position feedback.

<a id="state"></a>

## Main-loop behavior and tradeoffs

Page 0 checks face_flag, key, card, then fingerprints when no card is detected. Page 1 selects four management items; pages 2–5 add/delete cards/fingerprints. Keys wait 20 ms and block until release; response/failure screens also delay.

Startup retries AS608 handshake indefinitely. The 500-byte receive buffer and 8-bit RX_len differ; binary responses are searched with `strstr` without complete length/checksum verification. No explicit USART1 IDLE enable call was located; the ISR also compares SR against `UART_IT_IDLE`, requiring HAL-definition/device review. A historical demo does not prove the current port's receive handling.

OLED bit-bangs PB6/7, sends write address 0x78, and ignores ACK; this is not configured hardware I2C. RC522 also uses GPIO software SPI. Two processors do not establish electrical isolation or a security boundary.

<a id="implementation-status"></a>

## Verification and incomplete work

ARM GNU 13.3 `-fsyntax-only` passed all Core/Src C files. No linking/flashing occurred. K210 passed CPython syntax parsing, without executing Maix APIs, encrypted models, SD, camera, or LCD. No fresh recognition/timing/angle/power measurements.

The `.ioc` contains basic RCC/NVIC only, zero peripherals, and 8 MHz clocks, unlike handwritten 72 MHz/peripheral source. Regeneration is not a verified recovery method. Legacy toolchain I/O code is present; full linking still needs validation.

Historical materials/photos remain. Unsupported industrial/security/liveness, sub-100 ms, tamper alarm, quantization training, original minutiae algorithms, and FAR/FRR claims are removed. Priorities are protocol/storage/timeouts/authorization, then reproducible models/toolchain and physical evaluation. Source/photos/models are unchanged.
