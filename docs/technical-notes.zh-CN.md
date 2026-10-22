# Three-Mode Access Control Prototype — 技术说明

**简体中文** · [English](technical-notes.en.md) · [日本語](technical-notes.ja.md) · [项目首页](../README.md)

<a id="source-map"></a>

## 源码与版本依据

| 入口／模块 | 源码与责任 |
| :--- | :--- |
| 主流程 | [main.c](../STM32_Project/Core/Src/main.c)：初始化、六页菜单、三路判定、登记与删除。 |
| 串口 | [usart.c](../STM32_Project/Core/Src/usart.c)、[中断](../STM32_Project/Core/Src/stm32f1xx_it.c)：指纹接收和人脸标志。 |
| 设备 | [AS608.c](../STM32_Project/Core/Src/AS608.c)、[RC522.c](../STM32_Project/Core/Src/RC522.c)：命令封包及寄存器读写。 |
| UI／执行 | [OLED.c](../STM32_Project/Core/Src/OLED.c)、[key.c](../STM32_Project/Core/Src/key.c)、[Servo.c](../STM32_Project/Core/Src/Servo.c)、[tim.c](../STM32_Project/Core/Src/tim.c)。 |
| 持久化 | [myflash.c](../STM32_Project/Core/Src/myflash.c)、[常量](../STM32_Project/Core/Inc/myflash.h)。 |
| 视觉 | [boot.py](../K210_Project/boot.py)、[模型限制](../K210_Project/redme.md)。 |
| 工程 | [ioc](../STM32_Project/SmartAccessControlSystem.ioc)、[链接脚本](../STM32_Project/STM32F103C8TX_FLASH.ld)、`.cproject`。 |

当前维护版在课程 `7Project` 基础上修复，不再与归档逐字节一致；照片和模型保持原样，未将不同历史版本的功能拼接。

<a id="authentication"></a>

## 三种判定的实际范围

**刷卡：** 主循环只调用`PcdRequest`、`PcdAnticoll`并比较UID。防冲突读取四字节及BCC异或校验；驱动另有选卡、CRC计算、扇区认证／读写函数，但当前开锁不调用这些来证明加密认证，也未实现完整多卡二叉树遍历。

**指纹：** MCU封装EF01头、地址、包标识、长度、命令和加法校验和。特征和搜索在模块内部，不能把虚构的细节点图匹配公式写成MCU自研算法。录入两次生成缓冲1/2、Match、RegModel、StoreChar；UI使用0–3槽。搜索区间0起300项，成功且`mathscore>100`开锁，这不是百分比。

**人脸：** 调用KPU YOLO2检测、关键点网络、五点仿射对齐、特征网络、`face_encode/face_compare`。代码注释称196维，但缺少网络结构／训练和输出维度核验，不扩写成自主训练或确定的余弦归一化公式。阈值是SDK得分`>80`，不是余弦0.72，也不是80%准确率。无活体、防照片和FAR／FRR实验。

K210的本地登记和加载最多使用10个命名槽，修复清除函数的全局作用域；远程A/B/C输入仍未启用。人脸事件仍为ASCII `1`，确认是尽力发送；没有身份、序号、重放防护或安全认证，不能作为实际门锁产品。

<a id="storage-actuation"></a>

## 持久化与舵机

四个UID仍保存在 `0x08009000/04/08/0C`，每槽4字节；链接程序区限制为36 KiB，保留原UID页并使代码越界在链接时失败。物理Flash仍为64 KiB，页后空间暂不分配程序。写入只允许UID页内对齐范围，检查HAL返回值并向UI报告失败。没有事务或掉电恢复；刷写时仍需避免工具全片擦除。



按当前72 MHz定时器时钟、PSC719、ARR1999：

```math
T=\frac{(719+1)(1999+1)}{72\times10^6}=0.020\,s,\qquad t_{high}=CCR\times10\,\mu s
```

因此CCR50/175分别为0.5/1.75 ms，不能沿用旧文档CCR500/1500或0°／90°结论。`ControlServo`等待3 s后恢复，不读取锁舌、门磁或角度反馈。

<a id="state"></a>

## 主循环与工程取舍

page0依次检查face_flag、按键、卡片，未发现卡才调用指纹路径；page1为四项管理菜单，page2–5对应卡片／指纹添加删除。20 ms按键去抖后等待松开；应答和失败提示也含阻塞延时。

修复指纹二进制应答的有界长度、地址和累加校验检查；接收长度改为16位，ISR发布独立快照并显式启用IDLE，USART2改用非阻塞确认发送及单次重装。AS608启动最多尝试5次，缺席时保留人脸/卡片路径并禁用指纹操作。

OLED是PB6/7软件时序，发送0x78写地址且不检查ACK；不是已配置的硬件I2C。RC522同样GPIO模拟SPI。两芯片分工不等于电气隔离，更不构成安全认证。

<a id="implementation-status"></a>

## 验证与未完成内容

ARM GCC 13.3编译并链接通过：text 19,952 B、data 128 B、bss 4,760 B；8项独立应答解析用例经Clang编译为WebAssembly并在Node执行通过。K210仅做语法检查。没有烧录、外设测试或新增识别/性能结论。

`.ioc` 仅含基础RCC/NVIC，与手写72 MHz和外设配置不一致；重新生成仍不是已验证的恢复方式。构建通过不代表设备端串口时序已验证。

当前维护版在课程 `7Project` 基础上修复，不再与归档逐字节一致；照片和模型保持原样，未将不同历史版本的功能拼接。
