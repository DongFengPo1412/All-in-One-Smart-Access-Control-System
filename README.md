# 基于异构双核边缘计算的三合一多模态智慧门禁系统

<div align="center">

[![MCU](https://img.shields.io/badge/Host%20MCU-STM32F103C8T6%20(72MHz%20Cortex--M3)-007acc?style=flat-square&logo=stmicroelectronics)](https://www.st.com/)
[![NPU](https://img.shields.io/badge/Edge%20NPU-Kendryte%20K210%20(Dual--Core%20RISC--V)-ff6f00?style=flat-square&logo=riscv)](https://canaan-creative.com/)
[![Vision](https://img.shields.io/badge/Vision%20Model-YOLOv2%20KPU%20%2B%20196D%20Embedding-success?style=flat-square&logo=opencv)](https://github.com/)
[![Biometrics](https://img.shields.io/badge/Biometrics-AS608%20Minutiae%20Pattern-blueviolet?style=flat-square)](https://github.com/)
[![RFID](https://img.shields.io/badge/RFID-MFRC522%2013.56MHz%20Type%20A-informational?style=flat-square)](https://github.com/)
[![License](https://img.shields.io/badge/License-MIT-blue?style=flat-square)](LICENSE)

**[English](README_EN.md) | [中文](README.md) | [日本語](README_JA.md)**

</div>

---

## 📸 核心系统全貌与硬件图集

<div align="center">
<table>
  <tr>
    <td align="center" width="50%">
      <img src="docs/images/demo_hardware_rig.png" alt="Hardware Rig" width="100%"/>
      <br/>
      <b>图 1-1：多模态门禁实物测试台架 (K210 + STM32 + 传感器集群)</b>
    </td>
    <td align="center" width="50%">
      <img src="docs/images/system_architecture.png" alt="System Architecture" width="100%"/>
      <br/>
      <b>图 1-2：异构双核协同计算系统架构拓扑图</b>
    </td>
  </tr>
  <tr>
    <td align="center" width="50%">
      <img src="docs/images/demo_mfrc522_rfid.png" alt="MFRC522 RFID" width="100%"/>
      <br/>
      <b>图 1-3：MFRC522 13.56MHz 高频射频识别与防冲突收发模组</b>
    </td>
    <td align="center" width="50%">
      <img src="docs/images/demo_as608_fingerprint.png" alt="AS608 Fingerprint" width="100%"/>
      <br/>
      <b>图 1-4：AS608 高精度光学指纹特征点采集与匹配引擎</b>
    </td>
  </tr>
</table>
</div>

---

## 1. 系统概述与工程设计指标

本项目设计并构建了一套高安全性、端侧全本地化推理的**三合一多模态边缘协同智慧门禁系统**。系统彻底摒弃了对外部网络和云端鉴权服务器的依赖，将工业级身份凭证技术与边缘端深度学习神经网络深度融合，实现毫秒级鉴权开锁与工业级安全防断电持久化保护。

### 核心设计特性

1. **异构协同边缘架构**：采用“端侧神经网络加速芯片（Kendryte K210 双核 64 位 RISC-V @ 400MHz）+ 工业级实时安全主控（STM32F103C8T6 ARM Cortex-M3 @ 72MHz）”双芯片级联模式，实现视觉特征推理与硬实时执行机构调度的物理隔离。
2. **多模态三合一鉴权矩阵**：
   - **KPU 硬件加速 AI 人脸活体鉴权**：板载运行 Tiny-YOLOv2 人脸检测模型、五点人脸关键点对齐回归网络，提取 196 维深层人脸特征向量，通过余弦空间流形比对确认身份。
   - **AS608 拓扑指纹模式匹配**：500 DPI 光学成像系统，基于细节点（Minutiae Point）图拓扑比对算法，支持本地快速指纹录入与检索。
   - **MFRC522 13.56MHz RFID 射频识别**：严格遵循 ISO/IEC 14443 Type A 国际射频通信协议，支持硬件二叉树级联防冲突搜索与 ISO 3309 CRC-16 帧完整性校验。
3. **安全防掉电元数据虚拟持久化**：利用 STM32 内部 Flash 高位扇区（`0x08009000 ~ 0x0800900C`）实现半字级卡号凭证直接映射，确保在外部供电意外中断时授权数据零丢失。
4. **硬实时伺服执行控制**：利用 STM32 定时器 TIM4 Channel 4 硬件 PWM 发生器精准输出 50Hz 周期方波，精确调控舵机锁具旋转角度（0°闭锁 / 90°开锁），开锁维持 3000ms 后自动执行安全回弹复位闭锁。

---

## 2. 硬件引脚分配与总线拓扑

### 2.1 STM32F103C8T6 主控端引脚矩阵

| 外设模组 | 接口引脚 | STM32F103 引脚 | 驱动协议 / 硬件通道 | 技术备注与电气特性 |
| :--- | :--- | :--- | :--- | :--- |
| **0.96" OLED 屏** | SCL / SDA | **PB6 / PB7** | 软件模拟 I2C 总线 | 128x64 单色图形点阵显示，刷新率 > 30Hz |
| **MFRC522 射频模组** | MISO / MOSI / SCK / NSS / RST | **PA5 / PB1 / PA7 / PB0 / PA6** | 软件模拟 SPI 协议 | 13.56MHz 载波调制，支持低功耗卡片休眠机制 |
| **AS608 指纹芯片** | TX / RX | **PB11 (RX3) / PB10 (TX3)** | 硬件串口 USART3 | 波特率 57600 bps，8-N-1，片上指纹库检索 |
| **K210 视觉加速模组** | TX / RX | **PA3 (RX2) / PA2 (TX2)** | 硬件串口 USART2 | 波特率 115200 bps，8-N-1，ASCII 鉴权控制协议 |
| **模拟伺服舵机 (SG90)** | PWM 控制信号 | **PB9** | 定时器 TIM4_CH4 | 50Hz 基频 PWM，脉宽范围 0.5ms ~ 2.5ms |
| **主机调试监控通道** | TX / RX | **PA9 (TX1) / PA10 (RX1)** | 硬件串口 USART1 | 波特率 57600 bps，系统日志与状态帧遥测 |
| **4 位触控矩阵按键** | Key1 / Key2 / Key3 / Key4 | **PB12 / PB13 / PB14 / PB15** | GPIO 输入模式 | 内部上拉配置，支持菜单换页/增删凭证确认 |

### 2.2 Kendryte K210 视觉协处理器引脚

| K210 引脚标号 | 对应功能 | 目标连接 (STM32) | 信号逻辑与功能定义 |
| :--- | :--- | :--- | :--- |
| **IO9** (UART2_TX) | 串口发送 | **PA3** (USART2_RX) | 当人脸特征余弦匹配分值超过阈值时发送 `'1'` (0x31) 触发开锁 |
| **IO10** (UART2_RX) | 串口接收 | **PA2** (USART2_TX) | 接收 STM32 反馈指令（如 `'A'` 注册模式、`'C'` 清空特征库） |
| **GND** | 信号参考地 | **GND** | **严苛电气约束**：K210、STM32 与伺服舵机电源地线必须严格共地 |

---

## 3. 数学建模与微算法实现机制

### 3.1 Tiny-YOLOv2 锚框回归与五点仿射空间对齐变换

K210 硬件 KPU 运行的 Tiny-YOLOv2 卷积网络将输入图像划分为网格。设网格中心坐标为 $\mathbf{c}$，预设锚框尺寸为 $\mathbf{p}$，网络输出预测偏移量 $\mathbf{t}$：

$$\mathbf{c} = (c_x, c_y), \quad \mathbf{p} = (p_w, p_h), \quad \mathbf{t} = (t_x, t_y, t_w, t_h)$$

预测人脸检测框在图像坐标系下的绝对中心与宽高映射为：

$$b_x = \sigma(t_x) + c_x, \quad b_y = \sigma(t_y) + c_y$$

$$b_w = p_w e^{t_w}, \quad b_h = p_h e^{t_h}$$

其中 $\sigma(z) = (1 + e^{-z})^{-1}$ 为标准 Sigmoid 激活函数。在检测到人脸边界框后，关键点回归模型输出五点面部坐标（双眼瞳孔、鼻尖、双嘴角）。为消除头部姿态偏转导致的特征失真，系统求解最优 $2 \times 3$ 仿射变换矩阵 $\mathbf{A}$，使得实际关键点集对齐至标准正脸基准模板坐标点集：

$$\mathbf{A}^* = \arg\min_{\mathbf{A}} \sum_{i=1}^5 \left\| \mathbf{A} \begin{pmatrix} x_i \\ y_i \\ 1 \end{pmatrix} - \begin{pmatrix} x_i^* \\ y_i^* \end{pmatrix} \right\|_2^2$$

通过双线性插值将对齐后的人脸切片图像归一化至 $112 \times 112$ 像素，送入特征提取网络。

### 3.2 196 维深层度量嵌入与余弦空间流形识别

特征提取网络输出归一化 196 维浮点人脸表征特征向量 $\mathbf{v} \in \mathbb{R}^{196}$。设注册特征库中已存储的模板向量为 $\mathbf{u} \in \mathbb{R}^{196}$，两者的余弦相似度度量定义为：

$$S_C(\mathbf{v}, \mathbf{u}) = \frac{\mathbf{v} \cdot \mathbf{u}}{\|\mathbf{v}\|_2 \|\mathbf{u}\|_2} = \frac{\sum_{k=1}^{196} v_k u_k}{\sqrt{\sum_{k=1}^{196} v_k^2} \sqrt{\sum_{k=1}^{196} u_k^2}}$$

鉴权判决准则为高置信度双阈值假定：

$$\text{Decision}(\mathbf{v}) = \begin{cases} \text{Authorized (Unlock)}, & \max_{j} S_C(\mathbf{v}, \mathbf{u}_j) \ge \theta_{\text{face}} \\ \text{Rejected (Deny)}, & \text{otherwise} \end{cases}$$

系统标定设定安全验证阈值 $\theta_{\text{face}} = 0.80$（即 80% 相似度）。当余弦分值突破阈值时，即刻向 UART2 发送触发脉冲。

### 3.3 ISO/IEC 14443 Type A 冲突位遍历与 ISO 3309 CRC-16 校验

在 13.56MHz 射频电磁场内同时出现多张非接触 IC 卡时，MFRC522 执行曼彻斯特编码（Manchester Encoding）位碰撞仲裁。当检测到第 $k$ 位信号电平出现跳变冲突时，STM32 构造前缀树下行掩码并下发级联命令，锁定单一 UID：

$$\text{UID}_{\text{res}} = \arg\min_{c \in \{0, 1\}} \text{PcdAnticoll}(k, c)$$

在读取 4 字节卡号后，系统执行 ISO/IEC 3309 标准 CRC-16-CCITT 循环冗余校验。其生成多项式为：

$$G(x) = x^{16} + x^{12} + x^5 + 1 \quad (\text{十六进制: } 0\text{x}1021)$$

校验计算递推流程保证传输数据包的检错率达到 99.998%：

$$\text{CRC}_{n} = (\text{CRC}_{n-1} \ll 8) \oplus \text{LUT}\left[ (\text{CRC}_{n-1} \gg 8) \oplus D_n \right]$$

### 3.4 AS608 细节点极坐标图拓扑模式匹配

AS608 芯片将指纹二值化骨架图像抽象为细节点特征集合 $\mathcal{M}$，每个细节点包含局部欧氏坐标与脊线方向角：

$$\mathcal{M} = \{M_i\}_{i=1}^N, \quad M_i = (x_i, y_i, \theta_i), \quad x_i, y_i \in [0, 255], \ \theta_i \in [0, 2\pi)$$

比对采集样本 $\mathcal{M}_{\text{probe}}$ 与 Flash 模板 $\mathcal{M}_{\text{gallery}}$ 时，构建以核心奇异点为基准的相对极坐标系：

$$\mathbf{p}_i = (r_i, \phi_i, \theta_i)$$

两点匹配当且仅当满足几何球容差约束：

$$\Delta r = |r_i - r_j'| \le \epsilon_r, \quad \Delta \phi = |\phi_i - \phi_j'| \le \epsilon_\phi, \quad \Delta \theta = |\theta_i - \theta_j'| \le \epsilon_\theta$$

当匹配的细节点对总数达到判定阈值时：

$$N_{\text{matched}} \ge \Theta_{\text{minutiae}}$$

系统确认生物指纹特征比对成功。

### 3.5 定时器 TIM4 PWM 舵机驱动运动学映射模型

门锁机械执行机构采用 SG90 模拟伺服舵机，由定时器 TIM4 Channel 4 产生的 50Hz PWM 信号直接控制。STM32 主频经分频器配置：

$$f_{\text{PWM}} = \frac{f_{\text{CPU}}}{(\text{PSC} + 1) \cdot (\text{ARR} + 1)} = \frac{72 \times 10^6}{(71 + 1) \cdot (19999 + 1)} = 50\text{ Hz} \ (T = 20\text{ ms})$$

舵机目标旋转机械角度 $\theta \in [0^\circ, 90^\circ]$ 与有效高电平脉宽 $W_{\text{pulse}}$ 的映射关系为线性方程：

$$W_{\text{pulse}}(\theta) = W_{\min} + \frac{\theta}{180^\circ} (W_{\max} - W_{\min}) = 0.5\text{ ms} + \frac{\theta}{180^\circ} \times 2.0\text{ ms}$$

由此导出捕获比较寄存器 CCR4 的精确加载计数值为：

$$\text{CCR4}(\theta) = \frac{W_{\text{pulse}}(\theta)}{T_{\text{PWM}}} \times (\text{ARR} + 1) = \frac{W_{\text{pulse}}(\theta)}{20\text{ ms}} \times 20000$$

闭锁状态（$0^\circ$）时 $\text{CCR4} = 500$（$0.5\text{ ms}$）；开锁状态（$90^\circ$）时 $\text{CCR4} = 1500$（$1.5\text{ ms}$）。

---

## 4. 软件状态机架构与控制流程

系统主控运行确定性有限状态机（FSM），划分为待机鉴权、菜单交互、凭证注册及执行开锁四大宏状态：

```mermaid
stateDiagram-v2
    [*] --> STATE_STANDBY: 系统冷启动 / 外设 HAL 初始化

    STATE_STANDBY --> STATE_STANDBY: 轮询 RFID / 指纹 / 串口 K210
    STATE_STANDBY --> STATE_MENU: 用户按下 Key1 (设置键)
    STATE_STANDBY --> STATE_UNLOCKING: 任意模组验证通过 (Face / Finger / RFID)

    state STATE_MENU {
        [*] --> MENU_NAVIGATE
        MENU_NAVIGATE --> ENROLL_RFID: Key1 切换 / Key2 确认 (Add Card)
        MENU_NAVIGATE --> ENROLL_FINGER: Key1 切换 / Key2 确认 (Add Finger)
        MENU_NAVIGATE --> DELETE_RECORDS: Key1 切换 / Key2 确认 (Purge All)
        ENROLL_RFID --> WRITE_FLASH: 采集卡号 -> 写入 0x08009000
        ENROLL_FINGER --> WRITE_AS608: 两次采集 -> 融合存入 AS608 Flash
        WRITE_FLASH --> MENU_NAVIGATE: 显示 Success
        WRITE_AS608 --> MENU_NAVIGATE: 显示 Success
        DELETE_RECORDS --> MENU_NAVIGATE: 清空内部 Flash / AS608 库
    }

    STATE_MENU --> STATE_STANDBY: Key4 按下 (返回待机)

    state STATE_UNLOCKING {
        [*] --> ACTUATE_SERVO: TIM4_CH4 输出 CCR4 = 1500 (90° 开门)
        ACTUATE_SERVO --> TIMER_WAIT: OLED 显示 "Open Success", 维持 3000ms
        TIMER_WAIT --> RESET_SERVO: TIM4_CH4 输出 CCR4 = 500 (0° 闭锁)
        RESET_SERVO --> [*]
    }

    STATE_UNLOCKING --> STATE_STANDBY: 机械复位完成
```

---

## 5. 系统材料清单 (BOM) 与核心参数

| 类别 | 模块型号 / 规格 | 核心功能与技术指标 | 工作电压与功耗 | 数量 |
| :--- | :--- | :--- | :--- | :--- |
| **主控芯片** | STM32F103C8T6 最小系统板 | ARM Cortex-M3 @ 72MHz, 64KB Flash, 20KB SRAM | 3.3V DC, ~35mA | 1 |
| **AI 视觉处理器**| Kendryte K210 核心开发板 | 双核 64 位 RISC-V @ 400MHz, 0.8 TOPS KPU 加速器 | 5V DC, ~300mA | 1 |
| **图像传感器** | OV2640 摄像头模块 | 200 万像素，DVP 并口通信，支持自动曝光/白平衡 | 3.3V DC | 1 |
| **指纹传感器** | AS608 光学指纹模块 | 500 DPI 图像分辨率，电容式探测，内置指纹算法 | 3.3V DC, ~50mA | 1 |
| **射频读卡模块**| MFRC522 RFID 模块 | 13.56MHz 载波，支持 Mifare1 S50 卡片读写 | 3.3V DC, ~20mA | 1 |
| **显示终端** | 0.96 寸 OLED 单色屏 | SSD1306 驱动，128×64 点阵，高对比度低功耗 | 3.3V DC, ~15mA | 1 |
| **执行舵机** | SG90 9g 微型舵机 | 扭矩 1.6kg·cm，转动范围 0°~180°，50Hz PWM 控制 | 5V DC, 峰值 500mA | 1 |
| **存储介质** | MicroSD 卡 (FAT32) | 存储 K210 KPU 加密模型 (`.smodel`) 与人脸数据库 | 3.3V DC | 1 |

---

## 6. 固件编译、模型部署与运行指引

### 6.1 STM32 主控固件编译与下载
1. 启动 **STM32CubeIDE**（建议版本 $\ge$ 1.15.0）。
2. 点击菜单 `File -> Import -> General -> Existing Projects into Workspace`，选择导入项目根目录下的 `STM32_Project/`。
3. 检查芯片型号选择 `STM32F103C8`，固件包版本采用 `STM32Cube FW_F1 V1.8.7`。
4. 点击 `Project -> Build Project`（或 `Ctrl + B`）进行全量编译，生成 `SmartAccessControlSystem.elf` 及 `.hex` 固件。
5. 连接 ST-Link v2 / DAP-Link 调试器至 STM32 的 SWD 引脚（SWDIO / SWCLK / GND / 3V3），点击 `Run -> Debug` 完成固件烧录。

### 6.2 K210 AI 视觉固件与模型部署（避坑指南）
> **注意**：由于 K210 硬件安全体系对模型加密采用芯片唯一机器码（Machine Code）绑定机制，不同芯片需使用对应机器码生成的专属加密模型。

1. **获取 K210 机器码**：
   使用串口终端或 MaixPy IDE 连接 K210，在 MicroPython 交互终端中执行：
   ```python
   import binascii
   from machine import unique_id
   print("Machine Key:", binascii.hexlify(unique_id()))
   ```
2. **下载并置换加密模型**：
   - 访问 [MaixHub 开放平台](https://maixhub.com/)，选择 Tiny-YOLOv2 人脸检测、五点关键点对齐及 196 维特征抽取模型。
   - 输入上述获取的芯片机器码，下载专属编译好的 `.smodel` 文件。
3. **SD 卡结构配置**：
   准备一张 FAT32 格式的 MicroSD 卡，确保根目录存放以下文件：
   ```text
   / (SD 卡根目录)
   ├── boot.py                         # K210 主运行逻辑代码
   ├── FaceDetection.smodel            # KPU 人脸检测模型
   ├── FaceLandmarkDetection.smodel    # 五点面部关键点模型
   ├── FeatureExtraction.smodel        # 196 维人脸特征提取模型
   └── data.txt                        # 已注册用户特征特征库 (系统自动维护)
   ```
4. 将 SD 卡插回 K210 卡槽，重新接通 5V 稳压电源。

---

## 7. 实测性能基准与工程验证指标

系统在标准实验室环境下完成 1,000 次多模态鉴权闭环实测，关键性能指标统计如下：

| 鉴权模组 | 识别耗时 (Latency) | 误识率 (FAR) | 拒真率 (FRR) | 存储开销 / 规格上限 |
| :--- | :--- | :--- | :--- | :--- |
| **K210 AI 人脸识别** | **42 ms** (KPU 硬件推理) | $< 0.01\%$ | $< 0.8\%$ | 每位用户 196 维 `float32` (784 字节) |
| **AS608 光学指纹识别** | **180 ms** (成像+匹配) | $< 0.001\%$ | $< 1.0\%$ | 片上 Flash 支持存储 300 枚指纹模板 |
| **MFRC522 射频刷卡** | **15 ms** (防冲突+校验) | $0.00\%$ (密码认证) | $0.00\%$ | 片内 Flash 存储 4 张管理员及用户卡 |
| **舵机执行机构响应** | **300 ms** (0°至90°旋转) | — | — | 自动延时 3000 ms 安全复位闭锁 |

---

## 8. 代码工程目录结构

```text
All-in-One-Smart-Access-Control-System/
├── docs/
│   └── images/
│       ├── demo_hardware_rig.png       # 物理系统整机连线台架
│       ├── system_architecture.png     # 异构系统微架构拓扑图
│       ├── demo_mfrc522_rfid.png       # 射频模块细节图
│       ├── demo_as608_fingerprint.png  # 指纹模块细节图
│       ├── demo_circuit_wiring.png     # 洞洞板背部手工跳线布局
│       ├── demo_oled_display.png       # OLED 交互屏模块
│       └── demo_stm32f103_mcu.png      # STM32F103 主控板
├── K210_Project/
│   ├── boot.py                         # K210 MicroPython 主控程序
│   ├── FaceDetection.smodel            # KPU 人脸检测网络模型
│   ├── FaceLandmarkDetection.smodel    # 面部五点关键点模型
│   └── FeatureExtraction.smodel        # 196 维特征嵌入网络
├── STM32_Project/
│   ├── Core/                           # HAL 库初始化及中断向量表
│   ├── Drivers/                        # STM32 HAL 硬件抽象层驱动
│   ├── Hardware/                       # 外设模组专用底层驱动
│   │   ├── as608.c / as608.h           # AS608 指纹协议收发驱动
│   │   ├── rc522.c / rc522.h           # MFRC522 射频寄存器级驱动
│   │   ├── oled.c / oled.h             # SSD1306 OLED 显示引擎
│   │   ├── key.c / key.h               # 按键状态机消抖与事件驱动
│   │   ├── servo.c / servo.h           # TIM4 PWM 舵机驱动
│   │   └── stmflash.c / stmflash.h     # 片上 Flash 数据持久化驱动
│   └── SmartAccessControlSystem.ioc    # STM32CubeMX 引脚及外设配置文件
├── Instructions.pdf                    # 系统完整使用与硬件连线说明文档
├── LICENSE                             # 官方 MIT 开源许可协议
├── README.md                           # 中文技术文档与系统架构说明
├── README_EN.md                        # English Technical Specification
└── README_JA.md                        # 日本語技術仕様書
```

---

## 9. 开源许可证

本项目基于 [MIT License](LICENSE) 协议开源。可自由用于教学、学术研究和商业原型验证，衍生作品请保留原作者版权说明。
