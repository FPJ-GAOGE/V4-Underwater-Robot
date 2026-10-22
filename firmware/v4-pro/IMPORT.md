# V4-pro 固件来源、构建与实例差异

本目录保存 [FineSUB](https://gitee.com/gan-ju/FineSUB) 的 `v4pro1` 分支源码快照，基准提交为 `5a7832648206e522e9b8e0dff108638789f6afdd`。它对应上游提交说明「V4PRO1 粗调pid版」。另一台实例的 `V4pro2` 分支固定为 `847ff0309c54016365c303b686369af9d005db41`，通过 [差异补丁](variants/v4-pro1-to-pro2.patch) 保留。

飞书 V4-pro 制作资料引用 FineSUB，但没有指定具体分支。这里根据上游的两个明确命名分支保存可复查的参考，**不能据此认定某台现存潜器已经烧录其中任一提交**。`pro1`、`pro2` 是 V4-pro 内部实例差异；它们不是 V4 与 V4-pro 的两种型号。V4 的 IWIN-FINS / V4-ly 固件位于相邻目录。

## 导入范围与许可

- FineSUB 根目录 [LICENSE](LICENSE) 为 MPL-2.0。所有原有作者、版权头和各 BSP 的第三方许可证均保留。
- 保留所有原始源码、三个 BSP、CubeMX `.ioc`、MDK 工程、OpenOCD 配置与上游文档，未改控制算法、PID、通信、接线或安全状态机。
- 不随仓库发布任何层级的 `.idea/`、`.vscode/` 或 MDK 用户调试选项 `*.uvoptx`。这包括 `Devices/PressureSensor/.idea/` 的 5 个文件、`Services/Bus/.idea/` 的 4 个文件，以及 ETL 的 `include/etl/.vscode/settings.json`；来源清单已排除这 10 个 IDE 文件。子模块已展开为本目录中的实际依赖文件，所以不保留上游 `.gitmodules`，也不需要运行 `git submodule update`。
- CMSIS-DSP 固定 `4ee6da97c239071283d869de7c636d7621280e7c`：导入 `Include/`、`PrivateInclude/`、`Source/`、README 和 Apache-2.0 LICENSE。
- ETL 固定 `a12dbbd91174e895bdd5492826b0226b9668fc5f`：导入 `include/`、README、版本说明和 MIT LICENSE；排除上述嵌套 VS Code 配置，保留 `generators/` 内的 10 个 `.bat` 源码生成脚本。
- 依赖的示例、测试数据、文档生成工具和 Python 包没有复制；当前固件 CMake 引用的 C/C++ 依赖目录已包含。
- 完整来源、排除清单、逐文件 Git blob ID、SHA-256 与字节数见 [UPSTREAM.json](UPSTREAM.json)。文件直接从固定提交的 Git 对象导出，避免本机换行设置影响来源校验。它描述导入时的原始文件，不包含本文或补丁。

## 构建入口

V4-pro 使用 **RoboMaster A / STM32F427**。不要因源码中同时有 `Robomaster_C`、`MC_Board` 就选择另一块板的目标。

1. 安装 Keil MDK / Arm Compiler 6，并确保 `armclang.exe`、`armlink.exe`、`fromelf.exe` 可用。
2. 上游保留的 MDK 工程为 [BSP/Robomaster_A/MDK-ARM/Robomaster_A.uvprojx](BSP/Robomaster_A/MDK-ARM/Robomaster_A.uvprojx)。已检查其中 59 个文件引用均存在，但文件清单**没有 `TaskSUB.cpp`**，不能把它当作当前 V4-pro 应用已配置完整的工程；使用 MDK 前需要同步应用源码、头文件搜索路径和依赖设置。
3. CMake 入口为 [CMakeLists.txt](CMakeLists.txt)，要求 CMake 3.21 以上、C++17。该上游配置直接使用 Windows `.exe` 工具名；不能按普通 Linux GCC 项目直接构建。

在本目录、已配置 Arm Compiler 6 和 Ninja 的 Windows 终端中，可尝试上游 CMake 构建：

```powershell
cmake -S . -B build -G Ninja
cmake --build build --target Robomaster_A
```

这次整理未找到可用的 Arm Compiler 6，**未完成固件编译或下载验证**。原 CMake 后处理期望 `build/bin/Robomaster_A.elf`；若生成器输出名称不同，需核对链接产物与 `fromelf` 输入。本仓库保留上游构建定义，未将未经验证的修改写入控制固件。

## 控制与通信入口

| 项目 | 当前源码行为 | 位置 |
| --- | --- | --- |
| 上位机命令 | UART3，115200、8N1；`UARTBuffer<3,30>` 调用 `Streamer::Decode` | `Interface/Examples/TaskSUB.cpp`、`BSP/Robomaster_A/Core/Src/usart.c` |
| IMU | H30，UART7，921600、8N1；`UARTBuffer<7,100>` | 同上 |
| 推进器 | 8 路 DSHOT，启用双向 3D 模式与遥测 | `Devices/ESC_Motors/DSHOT_Motor.hpp` |
| 舵机 | 俯仰 PWM ID 9，偏航 PWM ID 10，50 Hz；A 板表中分别为 D、C | `TaskSUB.cpp`、`Robomaster_A.h` |
| 补光灯 | PWM ID 11、12；A 板表中分别为 B、A | 同上 |
| 水压计 | I2C2 → TCA9548A `0x70` → 0–3 通道；MS5837 地址 `0x76` / `0x77` | `TaskSUB.cpp` |
| 调度 | `Setup` 初始化压力阵列，`Loop` 轮询压力；TIM7 回调运行设备和任务 | `Interface/Interface.cpp` |

`Services/V5Streamer/Streamer.hpp` 严格接收 **28 字节**：

| 字节 | 含义 |
| --- | --- |
| 0 | 帧头 `0xAA` |
| 1–20 | 5 个 `float32`：yawRate、forwardSpeed、leftRightSpeed、downSpeed、upSpeed |
| 21–23 | 3 个 `uint8`：startButton、camFillLight、frameFillLight |
| 24–25 | 2 个 `int8`：camYaw、camPitch |
| 26–27 | 对前 26 字节计算的 CRC16/Modbus，低字节在前 |

STM32 按小端内存直接拷贝浮点字段。这里没有独立长度字节，也没有 V4-ly 的「设备地址 + 命令号」格式；不要混用。上游注释写的「24 字节」与实际 `length != 28` 判断不符，以代码为准。

当前解码器只接受一次回调中恰好一个完整帧；本次未补充分帧缓存。没有进行实机串口分包、断线、失控保护或推进器验证。

## V4-pro1 / V4-pro2 差异

两个分支的全部固件差异限于 `TaskSUB.cpp`、`IMUBase.hpp`、`V5_SUB.hpp`。另有一处 IDE 运行配置差异，不进入固件导入。

### 逻辑电机到 DSHOT ID

表中位置来自源码变量名称，索引为 `V5_SUBBuilder` 内 `motors[]` 的零基索引；**不等同于照片中的人工编号**。主板接头与 GPIO 的映射在 `BSP/Robomaster_A/Robomaster_A.h`，应一起核对。

| 数组索引 | 源码位置名 | pro1 DSHOT ID | pro2 DSHOT ID |
| --- | --- | ---: | ---: |
| 0 | LF Lower / 左前下 | 4 | 4 |
| 1 | LF Upper / 左前上 | 1 | 3 |
| 2 | LB Upper / 左后上 | 2 | 1 |
| 3 | LB Lower / 左后下 | 3 | 2 |
| 4 | RB Lower / 右后下 | 5 | 8 |
| 5 | RB Upper / 右后上 | 8 | 7 |
| 6 | RF Upper / 右前上 | 6 | 5 |
| 7 | RF Lower / 右前下 | 7 | 6 |

### 控制、姿态与水压

| 项目 | pro1 | pro2 |
| --- | --- | --- |
| IMU 安装补偿矩阵 | `diag(-1,-1,1)` | 单位阵 `diag(1,1,1)` |
| 深度用压力通道，零基 | 1 和 3 的平均 | 仅 3 |
| 压力初始化采样通道 | 1、3 | 3 |
| yaw 输入处理 | 原值乘 `0.001 × 0.4` 积分 | 绝对值 `<0.05` 归零，限幅到 `[-1,1]` 后乘 `0.001 × 0.8` 积分 |
| 竖向输出，motors `[1,2,5,6]` | `[-U0,+U1,-U2,-U3]` | `[+U0,-U1,+U2,+U3]` |
| 水平输出，motors `[0,3,4,7]` | `[+L0,-L1,-L2,+L3]` | `[+L0,-L1,+L2,-L3]` |
| PID 数值 | 保持原参数 | 仅 `0.00f` → `0.0f` 文本变化，数值未变 |

`U`、`L` 分别是源码的 `upperMotorOutPut`、`lowerMotorOutPut`。pro2 的 IMU 注释仍写「180 度旋转」，但矩阵实际是单位阵。这里如实记录该冲突，没有替代码假定安装姿态。

### 使用 pro2 补丁

本目录默认始终是 pro1。要生成 pro2 工作副本，先复制本目录到一个单独目录，然后在副本内执行：

```powershell
git init
git -c core.autocrlf=false apply --check variants/v4-pro1-to-pro2.patch
git -c core.autocrlf=false apply variants/v4-pro1-to-pro2.patch
```

`git init` 使补丁路径相对于这个独立副本，避免在上层聚合仓库里误解相对目录。命令级 `core.autocrlf=false` 保留上游 LF 换行，不修改个人 Git 配置。补丁只能施加一次；若已经改过上述三个文件，应先逐项审查冲突。施加后正文的逐文件哈希仍代表 pro1，变更后的三个文件以 `alternate_patched_files` 及 `alternate_commit` 为准。

两个实例的端口、IMU 安装方向、推力符号和压力通道必须成组核对；不能只套用电机序号表。

## 本次验证

- 1658 个来源文件与固定提交逐文件核对 SHA-256 和 Git blob ID，共 49,356,433 字节。
- 补丁在独立临时 Git 工作目录通过 `git apply --check`，施加后 3 个文件与固定 `V4pro2` 提交逐字节一致。
- 未运行推进器、未连接潜器、未烧写、未声称控制参数适用于另一台实机。
