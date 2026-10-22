# V4 控制固件：构建、配置与通信

本说明针对 [firmware/v4](../firmware/v4/IMPORT.md) 中的 IWIN-FINS `V4-ly` 固定提交 `42e5f16f5b3ad3e23fceb5eb60868340b365dfd5`。源码与用户指定上游保持一致，仅排除已记录的构建产物和厂商二进制手册。

飞书资料的 V4 方案使用 RM-C、树莓派与 PCA9685；本分支的工程目标明确写为 `STM32F407IGHx`。两项分别是系统资料与工程配置的事实，尚不构成对任意板卡的兼容性验证。V4-pro 的 RM-A、Jetson 与 FineSUB 是独立实现，不能通过把这里的 `Robot_Version` 改成某个数值来切换。

## 构建入口

### Keil MDK

1. 安装 Keil MDK，并准备工程记录的 ARM Compiler 6.15（ARMCLANG）及 `Keil.STM32F4xx_DFP.2.14.0` 设备包；使用其他版本时需重新验证。
2. 打开 [MDK-ARM/RM_Frame_C.uvprojx](../firmware/v4/MDK-ARM/RM_Frame_C.uvprojx)，目标名称为 `RM_Frame_C`。
3. 核对目标芯片 `STM32F407IGHx`、链接内存布局、下载器以及实际硬件连接后执行 Build。
4. 实际应用入口为 [Usermain.cpp](../firmware/v4/userCode/devices/Src/Usermain.cpp) 的 `main()`；[Src/main.c](../firmware/v4/Src/main.c) 保留系统时钟等生成配置。

本次只检查了主 Keil 工程引用的源文件全部存在，未执行 Keil 构建。上游另有 `CubeMX/MDK-ARM` 快照；不要把两份工程的相对路径或输出目录混在一起。

### CMake / GNU Arm

[CMakeLists.txt](../firmware/v4/CMakeLists.txt) 已配置 GNU Arm 交叉编译器。准备 CMake、Ninja 和带 `arm-none-eabi-` 前缀的 GNU Arm 工具链并将其加入 PATH 后，在本仓库根目录执行：

```sh
cmake -S firmware/v4 -B firmware/v4/build -G Ninja
cmake --build firmware/v4/build
```

预期目标名为 `RM_Frame_C.elf`，构建脚本还会转换出 `RM_Frame_C.hex` 与 `RM_Frame_C.bin`。使用的链接脚本是 [STM32F407IGHx_FLASH.ld](../firmware/v4/STM32F407IGHx_FLASH.ld)。CMake 声明最低版本 3.7，C 为 GNU99，C++ 为 C++11，CPU 为 Cortex-M4。

这是上游构建入口的操作说明，不是本次构建成功记录。当前 CMake 中软/硬浮点的两组示例开关均被注释，不应直接假设与 Keil 的浮点 ABI 配置相同。升级编译器、改用硬浮点或重新生成 CubeMX 工程后，应分别验证编译、链接、启动和外设行为。

## 运行结构

[ParamConfig.h](../firmware/v4/userCode/devices/Inc/ParamConfig.h) 声明 8 个推进器、4 个舵机、4 个水压计，控制频率为 100 Hz，串口缓存上限为 128 字节。[Usermain.cpp](../firmware/v4/userCode/devices/Src/Usermain.cpp) 中设备表的顺序同时定义串口地址：

| 地址 | 设备 | 当前状态 |
| --- | --- | --- |
| `0x00` | IMU | 在设备表内，TIM1 节拍下约 400 Hz 更新 |
| `0x01` | PressureSensor | 在 100 Hz 主循环处理；交替读取温度和压力 |
| `0x02` | Propeller_I2C | PCA9685 扩展 PWM 推进器 |
| `0x03` | Servo_I2C | PCA9685 扩展 PWM 舵机 |
| `0x04` | LED | 在设备表内 |

Sonar、Watchdog、Buzzer 有对象或实现，但当前没有全部纳入设备表。`MX_IWDG_Init()` 被注释。[IMU.cpp](../firmware/v4/userCode/devices/Src/IMU.cpp) 的初始化仍检查 BMI088，但 `Handle()` 当前调用 `h30_attitude_update()`，本地 BMI088 DMA / Mahony 更新路径被注释。H30 通过 USART1 接收；不能把“文件里有 BMI088 驱动”理解为当前姿态闭环只依赖板载 BMI088。

当前 H30 姿态映射为 `roll=NormalizeAngle(h30.roll+π)`、`pitch=-h30.pitch`、`yaw=-h30.yaw`，角速度直接取 H30 的 `wx/wy/wz`。这一映射必须连同实际 IMU 安装方向验证。

## V40 配置与修改位置

`Robot_Version` 在 [Usermain.cpp](../firmware/v4/userCode/devices/Src/Usermain.cpp) 中设为 `V40`。[ParamConfig.h](../firmware/v4/userCode/devices/Inc/ParamConfig.h) 的枚举仅有 V30、V31、V32、V33、V40；没有 V41 或 V4-pro 分支。

| 项目 | 当前值 / 行为 | 源码位置 |
| --- | --- | --- |
| 推进器内侧编号 | `{1,2,6,5}`，注释顺序左前、左后、右前、右后 | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| 推进器外侧编号 | `{0,3,7,4}`，相同位置顺序 | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| 正反桨符号 | `{1,-1,1,1,-1,-1,1,-1}`，按推进器序号 | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| 初始 PWM / 死区补偿 | 1590 μs；8 路补偿均为 50 | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| PWM 频率与输出范围 | 50 Hz；推进器发送阶段限幅 1000–2000 μs | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| 推力分配 | 固定 8×6 分配矩阵、6×8 合力矩阵；坐标注释为前、右、下及 roll/pitch/yaw | [Propeller.h](../firmware/v4/userCode/devices/Inc/Propeller.h) |
| 速度/力上限 | 平面各 20 N，航向速率 0.8 rad/s，深度速率内部数值 10 | [Propeller.h](../firmware/v4/userCode/devices/Inc/Propeller.h) |
| 舵机通道 | 连续 PCA9685 通道 8–11；初始化复制 `ID_V40`，实际中断发送按连续地址写入 | [Servo.cpp](../firmware/v4/userCode/devices/Src/Servo.cpp) |
| 舵机初始脉宽 | `{1550,1500,1500,1550}` μs | [Servo.cpp](../firmware/v4/userCode/devices/Src/Servo.cpp) |
| 舵机启动/停止 | 第 0、3 项设为 1650 / 1550 μs；发送阶段限幅 1000–2000 μs | [Servo.cpp](../firmware/v4/userCode/devices/Src/Servo.cpp) |
| V40 压力零偏 | `{1018.4,1020.0,1018.5,1019.1}` | [Sensor.h](../firmware/v4/userCode/devices/Inc/Sensor.h) |
| 压力传感器位置 | x=`{-12,12,12,-12}`，y=`{-5.4,-5.4,5.4,5.4}`，z 全零；原文要求按测量修改 | [Sensor.cpp](../firmware/v4/userCode/devices/Src/Sensor.cpp) |
| I2C 扩展 | I2C2；TCA9548A 通道 0–3 为水压计，通道 4 为 PCA9685 | [Sensor.cpp](../firmware/v4/userCode/devices/Src/Sensor.cpp)、[Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| I2C 地址写法 | TCA 宏 `0x70` 调用时左移；PCA 宏 `0x80` 已是 HAL 使用的移位地址；压力宏 `0xEC` | [Extension.h](../firmware/v4/userCode/devices/Inc/Extension.h)、[Sensor.h](../firmware/v4/userCode/devices/Inc/Sensor.h) |

`Robot_Version` 的实际 switch 用于选择水压零偏。推进器数组、推力矩阵和 PID 参数为固定全局配置；舵机初始化也固定 V40。仅更改枚举不会自动完成整机适配。V3.x 枚举是历史潜器配置，不能替代 V4 / V4-pro 型号对应表。

当前控制包含深度 PID、姿态外环、角速度内环、前馈和 ESO。PID 参数位于 `Propeller.cpp` 开头；前馈/ESO 的 `J/B/D/omega_o` 仍带“参数辨识”TODO。本次整理未调整任何增益、符号或模型参数。

## 串口接口与协议

参数以 [Src/usart.c](../firmware/v4/Src/usart.c) 为准：

| 接口 | 当前配置 | 用途 |
| --- | --- | --- |
| USART1 | 921600，8N1，无硬件流控 | H30 IMU 接收与解码 |
| USART6 | 115200；HAL 9 位字长、偶校验、1 停止位（通常配置上位机为 8E1） | 设备命令与文本输出 |
| USART3 | 100000，8N1 | 已初始化；本提交主命令分发不使用它 |

USART6 使用空闲 DMA 接收。回调仅在接收长度 `Size>=2` 时置标志；主循环读取 `RxBuffer6[0]` 为设备地址，范围 0–4，再由设备读取 `RxBuffer6[1]` 作为命令。没有统一帧头、长度、CRC、序号或应答格式；应避免把多条指令不加区分地连发到一次空闲事件中。

### 推进器命令（地址 `0x02`）

下面的“最小字节数”是处理函数所需数据量，包含地址和命令；当前固件并未逐条检查这个长度。16 位整数使用有符号小端编码。除闭环切换和空实现外，下列目标/运动命令均在 `flag_on` 为真时才处理。

| 命令 | 最小字节数 | 当前行为 |
| --- | --- | --- |
| `0x00` | 4 | `int16_le / 10.0` 写入深度目标 |
| `0x01` | 4 | `int16_le / 10.0` 累加到深度目标 |
| `0x02` | 4 | int16 限幅至 -99…99，再乘 `MAX_DEPTH_SPEED/100` 作为深度目标变化速率 |
| `0x10` | 2 | **翻转** `flag_on`；不是独立的开/关命令 |
| `0x20` | 2 | 已注册航向闭环命令，但处理函数为空 |
| `0x21` | 4 | int16 角度值，度转弧度后设置航向目标 |
| `0x22` | 4 | int16 角度增量，度转弧度后累加 |
| `0x23` | 4 | int16 限幅至 -99…99，再乘 `MAX_ROTATE_SPEED/100` 作为航向目标变化速率 |
| `0x30` | 4 | byte2、byte3 分别为有符号 int8 横向、纵向输入，限幅 -99…99 后映射为平面推力 |

例如 `02 00 2C 01` 的负载是整数 300，函数写入深度目标 30.0；这是编码示例，只有闭环已经开启时才会处理。不要重复发送 `02 10` 来“保证开启”，因为第二次会再次关闭。

深度字段注释为 cm，但 `CalcRealPressure()` 当前仅计算压差并滤波，没有显式使用水密度和重力加速度转换为实际深度。`data_depth` 是四路结果的平均值。上述 `/10` 是源码中的数值变换，不能据此保证实际深度单位或精度；需要按传感器和水体标定。

`CommandSetHorizontalStop()` 和 `CommandTest()` 虽然存在，但没有注册到命令表中；不为它们臆造可用命令码。关闭闭环后 `Handle()` 仍把各推进器 PWM 设回 `InitPWM=1590` 并发送，因此不能将其解释为断电或停止输出。1590 μs 是否静止取决于实际电调标定。

### 舵机命令（地址 `0x03`）

| 命令 | 字节数 | 当前行为 |
| --- | --- | --- |
| `0x00` | 2 | 第 0、3 项脉宽设为 1650 μs |
| `0x01` | 2 | 第 0、3 项脉宽设回 1550 μs |

函数命名为 StartMotor / StopMotor，但含义由当前机构决定；它们不是所有舵机通用的电源启停指令。

### 状态输出与历史命令

主循环当前约 10 Hz 输出 roll、pitch、yaw 的角度值（三位小数、逗号分隔、最后换行）。UART 错误时置 `error` 后停止这一正常角度输出路径；错误处理、I2C 错误等也可能向同一串口输出文本/数字，不能把每一行都当作三个角度值。

压力设备的 `Receive()` 仍匹配裸 `CA/VA`；但裸字符的首字节不在 0–4 地址范围，前置地址后又无法匹配首字节字符串，因此这两个命令无法按当前分发器正常调用。旧 `ON/OFF/PRO/VEL/MOT/WASD` 说明也不能直接作为当前推进器接口文档。历史文件保存在 [串口通信协议.txt](../firmware/v4/串口通信协议.txt) 与 [说明文档Readme.txt](../firmware/v4/说明文档Readme.txt) 供追溯。

## 已知限制与验证边界

这些是对固定源码的静态核查结果，本次导入没有修复或改变它们：

- **短帧与缓存一致性**：入口只要求两字节，多个处理函数会读取第 3、4 字节；长度不足时可能消费缓存残留。DMA 直接重用 `RxBuffer6`，主循环消费前没有帧快照或队列。
- **失联保护**：看门狗未启用，尚无证据证明默认具备通信超时后的自动停桨。现有关闭闭环命令也不是电气断电。
- **总线等待**：推进器和舵机处理存在无超时忙等；I2C 错误回调重初始化外设，但状态机重置语句被注释，异常情况下有停滞风险。
- **传感器初始化**：压力传感器 PROM/CRC 初始化有无上限循环；缺少器件或持续读失败时可能无法完成启动。
- **标定持久化**：水压校准函数中 Flash 写入被注释，不能保证校准值重启保存。
- **参数与坐标系**：推进器中位值、死区、正反桨、分配矩阵、IMU 轴向及水压位置必须一起按具体潜器核对；V4-pro 的参数不应复制到这里。
- **构建与实机**：已完成源码导入一致性和主 Keil 文件引用检查；未进行交叉编译、烧录、传感器台架或推进器测试。

复现顺序建议为：先完成目标编译和单板启动，再验证串口解析、传感器反馈与标定、逐通道执行器方向，最后分别验证深度环和姿态环。记录所用提交、工具链、实物接线与参数，才能把结果归属于一台具体潜器。
