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

This maintenance revision fixes the course `7Project` baseline and is no longer byte-identical to the archive. Photos/models are preserved and features from different historical variants are not combined.

<a id="authentication"></a>

## What the three checks do

**Card:** main calls `PcdRequest/PcdAnticoll` and compares UIDs. Level-one anticollision retrieves four bytes with XOR BCC. Selection/CRC/sector authentication/read-write routines exist in the driver, but the unlock path does not use them to establish cryptographic authentication or a complete multi-card binary-tree traversal.

**Fingerprint:** MCU forms EF01/address/type/length/command/additive-checksum packets. Features/search run inside AS608; imagined minutiae graph equations are not MCU algorithms. Enrollment captures buffers 1/2, Match, RegModel, StoreChar; UI slots are 0–3. Search starts at 0 over 300 entries; successful response and `mathscore>100` unlock. This is not a percentage.

**Face:** KPU YOLO2 detection, landmark network, five-point affine alignment, feature network,`face_encode/face_compare`. Comments say 196 dimensions, but architecture/training/output-shape evidence is absent; no original training or specific normalized cosine equation is claimed. SDK score `>80` is neither cosine 0.72 nor 80% accuracy. Liveness/photo resistance/FAR/FRR are not evaluated.

K210 local enrollment/loading is capped at ten named slots and clear now updates global lists. Remote A/B/C input remains disabled. Face events remain ASCII `1` with best-effort acknowledgement, without identity, sequence or replay protection; this is not an authenticated production lock.

<a id="storage-actuation"></a>

## Persistence and servo

Four UIDs retain `0x08009000/04/08/0C`, four bytes per slot. The linker limits program Flash to 36 KiB, preserving the original UID page and failing on program overflow. Physical capacity is still 64 KiB; space after that page is not allocated to code. Writes are restricted to aligned ranges within the UID page, check HAL status and report failure in the UI. Transactions and power-loss recovery are absent; avoid programmer mass erase when preserving enrollment.



For the configured 72 MHz timer clock, PSC 719, ARR 1999:

```math
T=\frac{(719+1)(1999+1)}{72\times10^6}=0.020\,s,\qquad t_{high}=CCR\times10\,\mu s
```

CCR 50/175 therefore produce 0.5/1.75 ms, not the old CCR 500/1500 values or calibrated 0°/90°. `ControlServo` waits 3 s then resets, with no door/bolt/position feedback.

<a id="state"></a>

## Main-loop behavior and tradeoffs

Page 0 checks face_flag, key, card, then fingerprints when no card is detected. Page 1 selects four management items; pages 2–5 add/delete cards/fingerprints. Keys wait 20 ms and block until release; response/failure screens also delay.

Fingerprint ACKs now have bounded length, address and additive-checksum validation. Receive length is 16-bit; the ISR publishes a separate snapshot and IDLE is enabled explicitly. USART2 uses nonblocking best-effort acknowledgement and a single receive rearm. Startup attempts AS608 at most five times; absence disables fingerprint operations while preserving card/face paths.

OLED bit-bangs PB6/7, sends write address 0x78, and ignores ACK; this is not configured hardware I2C. RC522 also uses GPIO software SPI. Two processors do not establish electrical isolation or a security boundary.

<a id="implementation-status"></a>

## Verification and incomplete work

ARM GCC 13.3 compilation and linking pass: text 19,952 B, data 128 B, bss 4,760 B. Eight independent ACK parser cases compiled with Clang to WebAssembly pass in Node. K210 receives syntax checking only. No flashing, peripheral tests or new recognition/performance results.

The `.ioc` contains basic RCC/NVIC only, zero peripherals, and 8 MHz clocks, unlike handwritten 72 MHz/peripheral source. Regeneration is not a verified recovery method. The ARM GCC link is verified; CubeIDE regeneration and physical UART timing are not.

This maintenance revision fixes the course `7Project` baseline and is no longer byte-identical to the archive. Photos/models are preserved and features from different historical variants are not combined.
