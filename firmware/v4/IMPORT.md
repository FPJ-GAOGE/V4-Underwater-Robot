# V4 固件导入记录

本目录来自用户指定的 [IWIN-FINS / V4-ly](https://gitee.com/superkinkin/IWIN-FINS/tree/V4-ly/)，上游固定提交为 `42e5f16f5b3ad3e23fceb5eb60868340b365dfd5`，整理日期为 2026-10-22。通过 `git -c core.autocrlf=false archive` 提取该提交中的已跟踪文件，关闭本机换行转换；未复制上游 `.git`，未改变控制算法、硬件参数、通信实现及源码编码。

此固件属于 V4 的 RM-C 方案资料。实际工程目标在 [Keil 工程](MDK-ARM/RM_Frame_C.uvprojx)中写为 `STM32F407IGHx`，不能据目录名称推定它能直接烧录到任意 RM 板卡。V4-pro 的 RM-A / FineSUB 固件是另一条实现，参见仓库中的型号对照说明；这里的 `V40` 不代表 V4-pro。

## 保留与排除范围

保留 347 份上游文件，包括应用源码、驱动、中间件、启动文件、链接脚本、CubeMX 配置、Keil 工程、CMake 配置、原说明及所有随附许可。保留文件约 14.9 MB，逐文件计算 Git blob 标识并与固定提交的原始 Git 对象核对一致。

排除以下非源码或非构建必需文件：

- `build/**`：上游已跟踪的 CMake 缓存、Ninja 文件、编译器探测结果和本机路径。
- `**/*.lst`、`**/*.map`、`**/*.lnp`、`**/*.dep`、`**/*.build_log.htm`：历史编译中间物和日志。
- `**/JLinkSettings.ini`：本机调试器设置。
- `Drivers/STM32F4xx_HAL_Driver/*.chm`：8 份 ST 厂商二进制用户手册，合计约 120.5 MB；可从固定上游提交另行获取。

保留 `.sct`、`.uvprojx`、`.uvoptx`、RTE 头文件及启动汇编等工程配置。未将其他本机工程中的代码拼入本目录。

## 许可与署名

上游根目录的 [LICENSE](LICENSE) 是 GNU GPL 第 3 版文本，原样保留。源码原作者标注、ST / Arm 等第三方版权头及子目录许可均保留；新增说明不替换这些原许可。

| 内容 | 原许可文件 |
| --- | --- |
| 上游项目 | [GPL v3 文本](LICENSE) |
| CMSIS | [Apache License 2.0](Drivers/CMSIS/LICENSE.txt) |
| ST CMSIS 设备支持 | [原许可说明](Drivers/CMSIS/Device/ST/STM32F4xx/LICENSE.txt) |
| STM32 HAL | [原许可说明，含 BSD-3-Clause 后备条款](Drivers/STM32F4xx_HAL_Driver/LICENSE.txt) |
| ST USB Device Library | [原许可说明与 SLA0044 文本](Middlewares/ST/STM32_USB_Device_Library/LICENSE.txt) |

第三方目录具有各自条件，不能把整套驱动和中间件重新声明为一种新许可证。公开发布前应核对所实际分发组件的许可组合；本次导入只保留并标明已有许可，未作重新授权。

## 构建入口与核验状态

- Keil 入口：[MDK-ARM/RM_Frame_C.uvprojx](MDK-ARM/RM_Frame_C.uvprojx)，记录的编译器为 ARM Compiler 6.15（ARMCLANG），设备包为 `Keil.STM32F4xx_DFP.2.14.0`。
- GCC 入口：[CMakeLists.txt](CMakeLists.txt)，使用 `arm-none-eabi-gcc/g++`、C++11、Cortex-M4、[STM32F407IGHx_FLASH.ld](STM32F407IGHx_FLASH.ld)，生成 ELF、HEX、BIN。
- CubeMX 入口：[RM_Frame_C.ioc](RM_Frame_C.ioc)。`CubeMX/MDK-ARM` 还保留一份上游工程快照，不与主 Keil 入口混用。
- 已检查主 Keil 工程的全部 `FilePath` 引用：导入目录中均存在。
- 本次未进行目标固件编译、烧录或推进器实机验证。源码归档一致性和文件引用检查不等于构建通过或硬件可运行。

完整参数、串口命令、构建步骤与限制见 [V4 固件说明](../../docs/firmware-v4.md)。

## 阅读旧说明时的注意事项

原 [说明文档Readme.txt](说明文档Readme.txt) 和 [串口通信协议.txt](串口通信协议.txt) 为历史资料，保留便于溯源。当前提交的真实行为包括：

- `Robot_Version=V40`；枚举只有 V30、V31、V32、V33、V40，没有 V41 或 V4-pro。
- USART6 通过设备地址和命令字节分发，不使用旧文本中的 `ON/OFF/PRO/VEL` 作为当前推进器入口。
- 推进器初始脉宽是 1590 μs；关闭闭环后仍输出初始脉宽，不是切断输出。
- 看门狗实例不在设备表中，`MX_IWDG_Init()` 被注释，不能声称默认具备失联自动停止或重启。
- 压力类仍比较裸 `CA/VA` 字符串，但与当前首字节地址分发不兼容；校准 Flash 写入也被注释。
- 枚举选择不是一套完整型号配置开关：推进器参数和舵机通道仍固定在 V40 方案。

这些差异在本次整理中被记录，未悄悄修改上游控制行为。
