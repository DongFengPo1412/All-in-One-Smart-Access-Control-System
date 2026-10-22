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

仓库与本地`7Project`的main、AS608、RC522、usart、tim、Servo和myflash字节一致；桌面两份归档boot也一致。`6SmartGate`较新但实现不同，不能仅按时间替换本仓库。

<a id="authentication"></a>

## 三种判定的实际范围

**刷卡：** 主循环只调用`PcdRequest`、`PcdAnticoll`并比较UID。防冲突读取四字节及BCC异或校验；驱动另有选卡、CRC计算、扇区认证／读写函数，但当前开锁不调用这些来证明加密认证，也未实现完整多卡二叉树遍历。

**指纹：** MCU封装EF01头、地址、包标识、长度、命令和加法校验和。特征和搜索在模块内部，不能把虚构的细节点图匹配公式写成MCU自研算法。录入两次生成缓冲1/2、Match、RegModel、StoreChar；UI使用0–3槽。搜索区间0起300项，成功且`mathscore>100`开锁，这不是百分比。

**人脸：** 调用KPU YOLO2检测、关键点网络、五点仿射对齐、特征网络、`face_encode/face_compare`。代码注释称196维，但缺少网络结构／训练和输出维度核验，不扩写成自主训练或确定的余弦归一化公式。阈值是SDK得分`>80`，不是余弦0.72，也不是80%准确率。无活体、防照片和FAR／FRR实验。

K210发送ASCII `1`；STM32 USART2回`OK`并置face_flag。发送方未检查OK，也没有身份、序号、CRC、重放防护。K210 UART接收被注释，A/B/C远程登记／清除无有效数据源；本地IO16按键登记可走通。SD清除函数也缺全局声明且调用被注释。名字表十项，登记列表未限十人，存在越界风险。

<a id="storage-actuation"></a>

## 持久化与舵机

四个UID起始地址是`0x08009000/04/08/0C`，每槽四字节以两个半字保存，槽0没有额外管理员权限。页读改写保留其他数据，但没有事务、断电恢复或返回状态验证。`STM32_FLASH_SIZE=0x10000`为字节，范围检查却再乘1024；这属于边界问题，不是容量扩展。

链接脚本仍给程序整个64 KiB Flash，没有保留UID页。需查看实际map避免代码与数据相撞。指纹、UID和人脸分别存模块、STM32 Flash、SD，不是统一安全存储。

按当前72 MHz定时器时钟、PSC719、ARR1999：

```math
T=\frac{(719+1)(1999+1)}{72\times10^6}=0.020\,s,\qquad t_{high}=CCR\times10\,\mu s
```

因此CCR50/175分别为0.5/1.75 ms，不能沿用旧文档CCR500/1500或0°／90°结论。`ControlServo`等待3 s后恢复，不读取锁舌、门磁或角度反馈。

<a id="state"></a>

## 主循环与工程取舍

page0依次检查face_flag、按键、卡片，未发现卡才调用指纹路径；page1为四项管理菜单，page2–5对应卡片／指纹添加删除。20 ms按键去抖后等待松开；应答和失败提示也含阻塞延时。

启动反复等待AS608握手成功，无总超时。指纹500字节接收与`RX_len`8位计数不匹配，`strstr`用于二进制应答且缺完整长度／校验验证；未找到明确使能USART1 IDLE中断的调用，ISR还把SR与`UART_IT_IDLE`常量比较，需结合HAL定义与实机排查。不能只凭历史演示宣称当前移植工程接收完全正确。

OLED是PB6/7软件时序，发送0x78写地址且不检查ACK；不是已配置的硬件I2C。RC522同样GPIO模拟SPI。两芯片分工不等于电气隔离，更不构成安全认证。

<a id="implementation-status"></a>

## 验证与未完成内容

本轮ARM GNU13.3对Core/Src全部C文件做`-fsyntax-only`检查通过；未链接／烧录。K210代码通过CPython语法解析，但Maix专用模块、加密模型、SD和实体相机均未执行。没有重新测量识别率、时延、角度或功耗。

`.ioc`仅含基础RCC/NVIC，外设数量为零、时钟记录8 MHz，与手写72 MHz及外设源码不符；不能把重新生成当作已验证恢复。混有旧工具链重定向代码，完整链接仍需实际验证。

保留课程资料与照片，删除工业级、高安全、活体、毫米／毫秒性能、防拆报警、量化训练、自主细节点算法和无依据FAR/FRR等完成声明。后续优先协议与存储、超时和权限，再补模型／工具链和实测。原源码、照片与模型没有改动。
