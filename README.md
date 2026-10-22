# IWIN-FINS V4 / V4-pro

面向水下机器人复现与维护的项目资料库：机械设计、采购清单、电气接线、嵌入式控制、上位机和部署说明。V4 与 V4-pro 分别保留各自的软件链路与硬件配置。

**当前为私有开源准备仓库。** 本次整理固定已有源码与设计资料，未改动控制算法，也未进行烧录或实机验证。不同目录沿用各自的许可；尚无明确许可的资料见 [许可与来源](THIRD_PARTY_NOTICES.md)。

![V4 设计预览](docs/images/v4-design.png)

## 先选型号

| | V4 | V4-pro |
| --- | --- | --- |
| 文档中的主控方案 | RoboMaster C 板 | RoboMaster A 板 |
| 舱内计算机 | 树莓派 4B | Jetson Orin Nano / NX，具体载板需核对 |
| 控制固件 | IWIN-FINS `V4-ly`，默认 V40 | FineSUB `v4pro1` 基准，附 `V4pro2` 差异补丁 |
| 推进器输出 | I²C → PCA9685 → 8 路 PWM | A 板 DSHOT → 2 块四合一电调 → 8 路无刷推进器 |
| 上层控制链路 | PC TCP 5000 → 树莓派 → UART6 | PC UDP 12345 → Jetson → UART3 |
| 固件命令串口 | 115200、8E1 | 115200、8N1 |
| 机械来源 | 2025-05-23 完整模型；2025-07-10 相机支架 | V4-PRO 制作文档中的 `V4pro.zip` |
| 详细说明 | [V4 硬件](docs/hardware-v4.md) · [V4 固件](docs/firmware-v4.md) | [V4-pro 硬件](docs/hardware-v4-pro.md) · [V4-pro 固件](firmware/v4-pro/IMPORT.md) |

**两个型号不能通过更改一个版本号互换固件。** PC 软件、串口帧格式、PWM 通道、IMU 方向及压力融合也有差别。先读 [完整型号差异](docs/variants.md) 和 [系统结构](docs/architecture.md)。

## 从这里开始

1. **制作机械部分**：查看 [机械文件与下载](hardware/README.md)、[BOM](docs/bom.md) 及对应型号的硬件说明。
2. **理解或编译控制代码**：V4 见 [firmware/v4](firmware/v4/IMPORT.md)；V4-pro 见 [firmware/v4-pro](firmware/v4-pro/IMPORT.md)。代码来源和提交均固定，可独立追溯。
3. **运行 PC / 舱内程序**：按 [使用与部署](docs/operation.md) 选择链路。V4 PC 的入口是 `software/v4-topside/pc_client.py`；V4-pro Jetson 的入口是 `software/v4-pro-jetson/upstream/V5StreamerNX.py`。
4. **核对首次下水条件**：先完成接线和无动力水密检查，再逐个验证传感器、推进器顺序与方向，最后闭环调试。参见对应硬件说明及 [已知缺口](docs/known-limitations.md)。

```text
firmware/
  v4/                         IWIN-FINS V4-ly 原始控制源码
  v4-pro/                     FineSUB pro1 源码、固定依赖与 pro2 补丁
software/
  v4-topside/                 V4 PC 手柄与曲线客户端
  v4-pro-jetson/              Jetson 转发、视频脚本及部署模板
  legacy-2025-communication/  历史图像通信示例（不属于当前控制链路）
hardware/
  v4/                        V4 STEP / DWG、机械爪、相机支架、接线原图
  v4-pro/                    V4-pro STEP / DWG、推力计算脚本
  assets-manifest.json        CAD 原包与提取文件的 SHA-256 清单
docs/                        型号对照、装配、使用与来源记录
tools/                       离线资料完整性检查
```

完整 SolidWorks 原包从 [机械源文件 Release](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/releases/tag/mechanical-source-2026-10-22) 下载；私有仓库需要登录有权限的 GitHub 账户。Git 中保留便于阅读与加工的中性格式，原包保留装配关系和原生工程文件。

## 当前交付边界

- V4 的 PC 客户端和控制固件已收录；实际树莓派 `joystick_control` 服务端未出现在所给仓库或附件中，因此不能仅靠本仓库从零复现整条 V4 链路。
- V4-pro 已收录固件与 Jetson 桥接脚本；制作资料提到的配套 Windows 手柄程序没有提供可确认的源码。V4 的 TCP 客户端不能代替它。
- 已核验源码导入、型号参数、CAD 校验值与 Python 静态语法；没有交叉编译、安装全部运行依赖或进行实机联调。[验证记录](docs/validation.md) 给出具体范围。
- 历史资料中有硬件参数冲突和实现缺陷，已集中列在 [已知缺口](docs/known-limitations.md)。仓库不包含设备登录密码、飞书认证文件、人员分工或经费记录。

## 来源与贡献

项目资料来自 IWIN-FINS 团队的飞书文档及其引用的上游仓库。详见 [来源记录](docs/sources.md)、[第三方许可](THIRD_PARTY_NOTICES.md) 和 [贡献约定](CONTRIBUTING.md)。

English: This repository organizes the existing V4 and V4-pro underwater robot designs and software as a private release-preparation snapshot. The two hardware/software stacks are documented separately. It does not claim a fully validated hardware build or a single license covering all imported material.
