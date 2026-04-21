# STM32F103C8 超声波模块 CubeMX 配置及使用说明

## 一、修改说明

**重要提示**：由于TIM2的4个通道已被用于4路舵机的PWM输出，本工程超声波模块使用 **TIM3** 替代TIM2进行输入捕获计时。

---

## 二、芯片选型
- 芯片型号: STM32F103C8Tx

---

## 三、系统核心配置

### 3.1 RCC 时钟配置
1. 进入 `Pinout & Configuration` -> `System Core` -> `RCC`
2. High Speed Clock (HSE): 选择 `Crystal/Ceramic Resonator`
3. 进入 `Clock Configuration` 页面
4. 配置系统时钟为 72MHz:
   - HSE: 8MHz
   - PLL Mul: x9
   - System Clock Mux: PLLCLK
   - HCLK: 72MHz
   - PCLK1: 36MHz (APB1)
   - PCLK2: 72MHz (APB2)

---

## 四、超声波模块引脚配置（保持不变）

### 4.1 引脚分配
| 模块引脚 | MCU引脚 | 功能模式       | 说明       |
|---------|---------|----------------|------------|
| TRIG    | PA6     | GPIO_Output    | 触发信号输出 |
| ECHO    | PA7     | GPIO_Input     | 回波信号输入 |

### 4.2 GPIO 详细配置
1. 点击 PA6 引脚 -> 选择 `GPIO_Output`
2. 点击 PA7 引脚 -> 选择 `GPIO_Input`
3. 进入 `System Core` -> `GPIO` 进行配置:

**PA6 (TRIG) 配置:**
- GPIO output level: Low
- GPIO mode: Output Push Pull
- GPIO Pull-up/Pull-down: No pull-up and no pull-down
- Maximum output speed: High
- User Label: ULTRASONIC_TRIG

**PA7 (ECHO) 配置:**
- GPIO mode: Input mode
- GPIO Pull-up/Pull-down: Pull-down
- User Label: ULTRASONIC_ECHO

---

## 五、定时器配置

### 5.1 TIM2 配置（舵机PWM输出，已配置完成）
- 通道1-4: PWM输出模式
- 用于4路舵机控制
- 引脚: PA0, PA1, PA2, PA3

### 5.2 TIM3 配置（超声波输入捕获，新增）
1. 进入 `Timers` -> `TIM3`
2. Clock Source: Internal Clock
3. Channel 1: Input Capture direct mode
4. **Parameter Settings**:
   - Prescaler (PSC): 71
   - Counter Mode: Up
   - Counter Period (ARR): 65535
   - Auto-reload preload: Enable
5. **NVIC Settings**:
   - TIM3 global interrupt: Enable
   - Preemption Priority: 2
   - Sub Priority: 0

### 定时器计算说明
- 定时器时钟: 72MHz
- PSC = 71, 计数频率 = 72MHz / (71+1) = 1MHz
- 每个计数 = 1us
- 最大测量时间: 65535us = 65.5ms
- 对应最大测量距离: ~11米

---

## 六、SYS 配置
1. 进入 `System Core` -> `SYS`
2. Debug: Serial Wire
3. Timebase Source: SysTick

---

## 七、NVIC 优先级分组
1. 进入 `System Core` -> `NVIC`
2. NVIC priority group: 4 bits for preemption, 0 bits for subpriority

---

## 八、项目设置
1. 进入 `Project Manager` -> `Project`
2. Toolchain / IDE: MDK-ARM V5
3. 进入 `Code Generator`
4. 勾选 "Generate peripheral initialization as a pair of '.c/.h' files per peripheral"

---

## 九、驱动代码部署

### 9.1 文件位置
驱动代码已放置在 `Drivers/Ultrasonic/` 目录下：
```
Drivers/Ultrasonic/
├── ultrasonic.h                    # 头文件
├── ultrasonic.c                    # 源文件
└── CubeMX_超声波模块配置及使用说明.md  # 本文档
```

### 9.2 MDK工程添加文件
1. 在MDK-ARM工程中右键点击Drivers组
2. 选择 "Add Existing Files to Group..."
3. 添加 `Drivers/Ultrasonic/ultrasonic.c`
4. 添加头文件路径:
   - 点击魔术棒 -> C/C++ -> Include Paths
   - 添加 `../Drivers/Ultrasonic`

---

## 十、示例代码使用

### 10.1 简单轮询方式（推荐新手使用）

```c
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "oled.h"
#include "ultrasonic.h"

float distance = 0;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();    // 初始化TIM3
    
    OLED_Init();
    Ultrasonic_Init(); // 超声波模块初始化
    
    char buf[32];
    
    while(1)
    {
        // 读取距离（轮询方式，阻塞）
        distance = Ultrasonic_GetDistance_Simple();
        
        if(distance < 400.0f)
        {
            sprintf(buf, "Dist: %.1f cm", distance);
            OLED_ShowString(2, 1, buf);
        }
        else
        {
            OLED_ShowString(2, 1, "OUT OF RANGE");
        }
        
        HAL_Delay(100);
    }
}
```

### 10.2 输入捕获方式（精度更高，非阻塞）

```c
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "oled.h"
#include "ultrasonic.h"

float distance = 0;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    
    OLED_Init();
    Ultrasonic_Init();
    
    while(1)
    {
        // 触发测量
        Ultrasonic_Trigger();
        
        // 等待测量完成
        HAL_Delay(50);
        
        // 获取距离
        distance = Ultrasonic_GetDistance();
        
        if(distance >= 0)
        {
            char buf[32];
            sprintf(buf, "Dist: %.1f cm", distance);
            OLED_ShowString(2, 1, buf);
        }
        
        HAL_Delay(50);
    }
}

// TIM输入捕获回调函数（需在main.c中添加）
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    Ultrasonic_TIM_IC_CaptureCallback(htim);
}
```

### 10.3 在当前main.c中集成测试

在现有main.c中修改，添加超声波测试功能：

```c
/* 在main.c的Private includes中添加 */
#include "ultrasonic.h"

/* 在main函数中，初始化后添加 */
Ultrasonic_Init();

/* 在主循环while(1)中添加按键测试 */
// 按键3：测试超声波（如果有按键的话）
if(KeyNum == 3)
{
    float dist = Ultrasonic_GetDistance_Simple();
    char buf[32];
    sprintf(buf, "Dist: %.1f cm", dist);
    OLED_Clear();
    OLED_ShowString(2, 1, "Ultrasonic Test:");
    OLED_ShowString(3, 1, buf);
    OLED_Update();
}
```

---

## 十一、API 函数说明

| 函数名 | 说明 | 参数 | 返回值 |
|--------|------|------|--------|
| `Ultrasonic_Init()` | 初始化超声波模块 | 无 | 无 |
| `Ultrasonic_Trigger()` | 触发一次测量（输入捕获模式） | 无 | 无 |
| `Ultrasonic_GetDistance()` | 获取测量结果（非阻塞） | 无 | 距离(cm)，-1表示测量中 |
| `Ultrasonic_GetDistance_Simple()` | 简单轮询读取距离（阻塞） | 无 | 距离(cm)，999表示超时 |
| `Ultrasonic_TIM_IC_CaptureCallback()` | TIM输入捕获中断回调 | htim: 定时器句柄 | 无 |

---

## 十二、超声波工作原理

1. **触发**：TRIG引脚输出至少10us的高电平
2. **发射**：超声波模块发射8个40kHz的脉冲
3. **等待回波**：ECHO引脚变高
4. **计时**：测量ECHO高电平持续时间
5. **计算距离**：距离(cm) = 时间(us) * 0.034 cm/us / 2
   - 0.034 cm/us: 声速
   - 除以2: 往返时间

---

## 十三、注意事项

1. **TIM3冲突**：TIM3通道1(PA6)虽然被配置为输入捕获，但超声波实际使用PA6作为GPIO输出触发信号，不冲突
2. **测量范围**：有效测量范围 2cm - 400cm
3. **测量频率**：建议两次测量间隔至少 60ms
4. **供电**：超声波模块请使用5V供电，3.3V供电可能导致测距不准
5. **中断回调**：使用输入捕获模式时，必须在main.c中实现`HAL_TIM_IC_CaptureCallback`并调用超声波的回调函数
