# Ultrasonic-Sensor-Module_STM32

基于STM32F103C8T6的HC-SR04超声波测距模块工程

---

## 📋 目录

- [工程概述](#工程概述)
- [硬件连接](#硬件连接)
- [CubeMX配置详细指导](#cubemx配置详细指导)
  - [RCC时钟配置](#rcc时钟配置)
  - [引脚定义与GPIO配置](#引脚定义与gpio配置)
  - [定时器配置详解与计算推导](#定时器配置详解与计算推导)
- [超声波模块配置注意事项](#超声波模块配置注意事项)
- [使用指导](#使用指导)
- [代码文件索引](#代码文件索引)

---

## 工程概述

本工程实现了基于STM32F103C8T6主控芯片的HC-SR04超声波测距功能，主要特性：

- **测量范围**：2cm ~ 400cm
- **测量精度**：±0.1cm（TIM3硬件精确计时）
- **显示单位**：支持厘米(cm)和毫米(mm)
- **状态指示**：根据距离显示Too close!/Attention!/Normal/Far四种状态
- **外设集成**：OLED显示、按键交互

---

## 硬件连接

### HC-SR04超声波模块

| 模块引脚 | STM32引脚 | 功能说明 | 注意事项 |
|---------|----------|----------|---------|
| VCC | 5V | 电源 | **必须5V！3.3V不能正常工作！** |
| GND | GND | 地 | |
| TRIG | PA6 | 触发信号输出 | 推挽输出 |
| ECHO | PA7 | 回波信号输入 | 下拉输入 |

### 其他外设

| 外设 | STM32引脚 | 功能 |
|-----|----------|------|
| OLED SCL | PB8 | I2C时钟 |
| OLED SDA | PB9 | I2C数据 |
| Key1 | PB13 | 测量单位cm |
| Key2 | PB15 | 测量单位mm |

---

## CubeMX配置详细指导

### RCC时钟配置

1. **HSE时钟源配置**：
   - RCC → HSE → Crystal/Ceramic Resonator
   - Clock Configuration → 系统时钟配置为72MHz
   - **重要**：APB1预分频器设置为/2，TIM2-TIM7时钟为72MHz

---

### 引脚定义与GPIO配置

在CubeMX的Pinout & Configuration选项卡中配置以下引脚：

| 引脚 | 功能 | 模式 | 上拉/下拉 | 速度 |
|-----|------|------|----------|------|
| PA6 | TRIG | GPIO_Output | No pull-up/down | High |
| PA7 | ECHO | GPIO_Input | Pull-down | - |
| PB8 | OLED_SCL | GPIO_Output (OD) | No pull-up/down | High |
| PB9 | OLED_SDA | GPIO_Output (OD) | No pull-up/down | High |
| PB13 | Key1 | GPIO_Input | Pull-up | - |
| PB15 | Key2 | GPIO_Input | Pull-up | - |

---

### 定时器配置详解与计算推导

#### TIM3 - 超声波精确计时（核心）

**用途**：用于测量ECHO引脚高电平持续时间，实现us级精确计时

**配置参数**：
```
Prescaler (PSC) = 72 - 1 = 71
Counter Mode    = Up
Period (ARR)    = 65535
Clock Division  = DIV1
```

📐 **配置计算推导过程**：

**第一步：确定定时器时钟频率**
- SYSCLK = 72MHz
- APB1预分频 = 2 → PCLK1 = 36MHz
- TIM2-TIM7时钟倍增 = ×2 → **TIM3时钟 = 72MHz**

**第二步：计算PSC实现1us计数周期**
```
目标：计数器每+1  = 1微秒(us)
公式：计数频率 = TIM时钟 / (PSC + 1)

需要计数频率 = 1MHz = 1次/us

所以：
72MHz / (PSC + 1) = 1MHz
PSC + 1 = 72MHz / 1MHz = 72
PSC = 72 - 1 = 71 ✓
```

**第三步：验证最大计时范围**
```
最大计数值 = ARR = 65535
最大计时时间 = 65535 × 1us = 65.535ms

HC-SR04最大测量距离400cm对应时间：
距离公式：距离(cm) = 时间(us) / 58
400cm × 58 = 23200us = 23.2ms

65.535ms > 25ms(超时设置) ✓ 计时范围足够
```

#### TIM2 - PWM定时器（预留扩展）

**用途**：4路PWM输出，可用于舵机、电机等控制

**配置参数**：
```
Prescaler (PSC) = 72 - 1 = 71
Counter Mode    = Up
Period (ARR)    = 20000 - 1 = 19999
```

📐 **PWM配置计算推导**：

```
目标：标准舵机频率50Hz，周期20ms

计数频率 = 72MHz / (71 + 1) = 1MHz = 1us/次

需要计数20000次达到20ms：
20000 × 1us = 20ms → 频率 = 1/20ms = 50Hz ✓

ARR = 20000 - 1 = 19999
```

---

## 超声波模块配置注意事项

### ⚠️ 硬件注意事项

1. **供电电压必须5V**
   - HC-SR04内部包含模拟电路，3.3V供电会导致发射功率不足
   - 现象：测量距离短、不稳定、完全无响应

2. **ECHO引脚电平兼容**
   - ECHO输出5V电平，但STM32引脚5V容忍，可以直接连接
   - 不需要额外电平转换电路

3. **布线建议**
   - TRIG和ECHO线尽量短，减少干扰
   - 远离电机、电源等噪声源

### ⚠️ 软件注意事项

1. **TIM3必须手动启动**
   - **大坑提醒**：CubeMX生成的代码只初始化定时器，不启动！
   - 必须调用：`HAL_TIM_Base_Start(&htim3);`
   - 代码位置：[ultrasonic.c:26](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Ultrasonic/ultrasonic.c#L26)

2. **触发脉冲宽度**
   - 数据手册要求≥10us，代码使用15us留有裕量
   - 触发前先拉低2us，保证脉冲干净

3. **超时处理**
   - 最大超时设置25000us（25ms），对应约431cm
   - 超过400cm返回999.0表示测量失败

---

## 使用指导

### 快速开始

1. **上电初始化**
   - OLED显示欢迎界面，提示"Press any key..."

2. **按键1 - 厘米模式**
   - 按Key1(PB13)开始测量，显示单位cm
   - 显示范围：2.0cm ~ 400.0cm
   - 调用函数：[Ultrasonic_Read_cm()](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Ultrasonic/ultrasonic.c#L41)

3. **按键2 - 毫米模式**
   - 按Key2(PB15)开始测量，显示单位mm
   - 显示范围：20.0mm ~ 4000.0mm
   - 调用函数：[Ultrasonic_Read_mm()](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Ultrasonic/ultrasonic.c#L108)

---

## 🆕 TIM输入捕获模式使用指导（B站Keysking方案）

### 模式对比

| 特性 | 软件查询模式（默认） | 输入捕获中断模式 |
|-----|-------------------|----------------|
| ECHO接线 | PA7 | **PB4** |
| 工作方式 | 软件循环查询电平 | 硬件自动捕获+中断 |
| CPU占用 | 阻塞，占用CPU约25ms | 非阻塞，完全不占用 |
| 精度 | 高（有极小查询延迟） | 极高（硬件级精度） |
| 适用场景 | 简单应用、学习 | 高性能、操作系统 |

### 硬件接线变更（重要！）

**使用输入捕获模式必须修改接线！**

| 模块引脚 | STM32引脚 | 功能说明 |
|---------|----------|----------|
| VCC | 5V | 必须5V供电 |
| GND | GND | 地 |
| TRIG | PA6 | 触发信号（不变） |
| ECHO | **PB4** | 回波信号输入TIM3通道1 |

### 输入捕获原理解析

**【PWM输入模式核心设计】**
这是B站Keysking教程的精髓：

1. **双通道复用技术**：
   - TIM3_CH1(PB4) = 直接通道，捕获**上升沿**
   - TIM3_CH2 = 间接通道，捕获**下降沿**
   - ✨ **两个通道共享同一个输入引脚！**

2. **从模式-复位模式**：
   - 检测到上升沿时，硬件**自动清零计数器**！
   - 不需要软件手动清零！
   - 所以下降沿捕获值 = 高电平持续时间

3. **中断触发点**：
   - 只在CH2下降沿进入中断
   - 同时读取CH1和CH2的捕获值
   - 一次中断完成全部计算！

### 使用步骤

#### 1. 初始化输入捕获模式
```c
// main函数初始化中调用，替代原来的 Ultrasonic_Init()
Ultrasonic_IC_Init();
```
位置：[ultrasonic.c:180](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Ultrasonic/ultrasonic.c#L180)

#### 2. 触发一次测量（非阻塞！）
```c
// 发送触发脉冲，立即返回
// 可以在定时器中断里周期性调用
Ultrasonic_IC_Trigger();
```
位置：[ultrasonic.c:197](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Ultrasonic/ultrasonic.c#L197)

#### 3. 读取测量结果（随时读！）
```c
// 触发后约25ms即可读取
// 直接返回全局变量，无任何等待
float distance = Ultrasonic_IC_Read_cm();
// 或毫米单位
float distance_mm = Ultrasonic_IC_Read_mm();
```

### 核心代码位置

| 功能 | 文件 | 跳转 |
|-----|------|------|
| 输入捕获中断回调 | tim.c | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Core/Src/tim.c#L67) |
| 启动输入捕获引擎 | tim.c | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Core/Src/tim.c#L352) |
| 驱动封装函数 | ultrasonic.c | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Ultrasonic/ultrasonic.c#L173) |

### ⚠️ 输入捕获注意事项

1. **CubeMX配置检查**：
   - TIM3通道1：PWM输入模式（不是普通输入捕获！）
   - 从模式(SMS)：复位模式 Reset Mode
   - 触发源(TS)：TI1FP1
   - 通道1极性：上升沿
   - 通道2极性：下降沿

2. **中断优先级**：
   - TIM3中断优先级建议设置为1或2
   - 确保比系统中断低，但比普通任务高

3. **测量周期**：
   - 两次触发间隔建议 ≥ 60ms
   - 避免前一次回波干扰下一次测量

---

### 距离状态说明

| 距离范围 | 状态提示 | 说明 |
|---------|---------|------|
| < 10cm | Too close! | 过近预警 |
| 10cm ~ 30cm | Attention! | 注意距离 |
| 30cm ~ 100cm | Normal | 正常范围 |
| > 100cm | Far | 较远范围 |

### API接口说明

#### 1. 初始化函数
```c
void Ultrasonic_Init(void);
```
位置：[ultrasonic.c:22](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Ultrasonic/ultrasonic.c#L22)
- 启动TIM3计数器
- TRIG引脚初始化为低电平
- 等待模块稳定

#### 2. 厘米测量
```c
float Ultrasonic_Read_cm(void);
```
返回值：
- 成功：2.0 ~ 400.0 (cm)
- 失败：999.0

#### 3. 毫米测量
```c
float Ultrasonic_Read_mm(void);
```
返回值：
- 成功：20.0 ~ 4000.0 (mm)
- 失败：9999.0

### 📐 测距原理解析

**物理公式推导**：
1. 声波在空气中速度 ≈ **343m/s** (20°C)
2. 声波往返距离 = 声速 × 时间
3. **实际距离 = 往返距离 ÷ 2**

**厘米单位公式推导**：
```
距离(cm) = (34300 cm/s × t) ÷ 2
         = 17150 × t
         = 17150 × (t_us × 10^-6)
         = t_us / 58.3 ≈ **t_us / 58**
```

**毫米单位公式推导**：
```
距离(mm) = t_us / 5.8
```

---

## 代码文件索引

### 核心驱动

| 文件 | 说明 | 跳转链接 |
|-----|------|---------|
| ultrasonic.c | 超声波核心测距实现 | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Ultrasonic/ultrasonic.c) |
| ultrasonic.h | 超声波驱动头文件 | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Ultrasonic/ultrasonic.h) |
| main.c | 主程序、按键处理、OLED显示 | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Core/Src/main.c) |

### 硬件配置

| 文件 | 说明 | 跳转链接 |
|-----|------|---------|
| tim.c | TIM2、TIM3定时器配置 | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Core/Src/tim.c) |
| gpio.c | GPIO引脚初始化配置 | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Core/Src/gpio.c) |
| main.h | 引脚宏定义 | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Core/Inc/main.h) |

### 工具驱动

| 文件 | 说明 | 跳转链接 |
|-----|------|---------|
| delay.c | us级延时函数 | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Delay/delay.c) |
| key.c | 按键驱动 | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/Key/key.c) |
| oled.c | OLED显示屏驱动 | [→](file:///d:/Projects_chj/OptoElec-Embedded-Projects/Ultrasonic-Sensor-Module_STM32/Drivers/OLED/oled.c) |

---

## ✅ 工程编译说明

- IDE：Keil MDK-ARM 5
- 目标芯片：STM32F103C8T6
- 编译入口：`MDK-ARM/STM32F1Dog_HAL.uvprojx`
- hex输出：`MDK-ARM/STM32F1Dog_HAL/STM32F1Dog_HAL.hex`
