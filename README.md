# IWIN-FINS V4 / V4-pro

**中文 / English** · [English edition](README.en.md) · [BOM 下载 / Download](hardware/bom/V4-V4pro-BOM.xlsx) · [型号差异 / Variants](docs/variants.md)

V4 / V4-pro 是 IWIN-FINS 水下机器人项目的两个硬件与软件方案。本仓库整理机械设计、BOM、电气接线、控制固件、上位机及制作与部署说明，供复现、维护和后续开发使用。

V4 and V4-pro are two hardware and software configurations in the IWIN-FINS underwater robot project. This repository brings together mechanical designs, bills of materials, wiring, controller firmware, topside software, and build and deployment documentation for reproduction, maintenance, and further development.

**当前仓库保持私有。** 各组件保留原有许可；已取得资料的范围、缺失程序及未完成的验证均在文档中注明。

**This repository remains private.** Components retain their original licenses. Available material, missing programs, and outstanding validation are documented separately.

![V4 设计预览 / V4 design preview](docs/images/v4-design.png)

## 项目概览 / Overview

两种潜器均采用八推进器方案，但主控、舱内计算机、电机驱动及控制协议不同。文档分别说明 V4 与 V4-pro；Pro 的两个固件分支也单独对照。

Both vehicles use eight thrusters, with different controllers, onboard computers, motor drivers, and control protocols. V4 and V4-pro are documented separately, including a comparison of the two Pro firmware branches.

| 项目 / Item | V4 | V4-pro |
| --- | --- | --- |
| 主控 / Controller | RoboMaster C 板 / Type C board | RoboMaster A 板 / Type A board |
| 舱内计算机 / Onboard computer | Raspberry Pi 4B | Jetson Orin Nano / NX，载板需核对 / verify carrier board |
| 固件 / Firmware | IWIN-FINS `V4-ly`，默认 / default V40 | FineSUB `v4pro1` + `V4pro2` 差异补丁 / variant patch |
| 推进器驱动 / Thruster drive | I²C → PCA9685 → 8 路 PWM / 8 PWM channels | DSHOT → 2 块四合一电调 / two 4-in-1 ESCs → 8 motors |
| 控制链路 / Control link | PC TCP 5000 → Raspberry Pi → UART6 | PC UDP 12345 → Jetson → UART3 |
| 命令串口 / Command UART | 115200, 8E1 | 115200, 8N1 |
| 机械版本 / Mechanical sources | 2025-05-23 完整模型 / full model; 2025-07-10 相机支架 / camera mount | Pro 制作文档中的 / build-document archive `V4pro.zip` |

两个型号不能通过更改版本号互换固件。推进器线制、串口帧、传感器坐标系、供电和控制增益需与实物匹配。详见 [中文型号对照](docs/variants.md) / [English variant comparison](docs/variants.en.md)。原资料未提供完整可复核的性能报告，因此此处不填写额定潜深、续航或速度。

The firmware is not interchangeable by changing a version flag. Thruster wiring, UART frames, sensor axes, power rails, and control gains must match the actual vehicle. Rated depth, endurance, and speed are not specified because the supplied material lacks a complete, verifiable test record for these builds.

## BOM 材料清单 / Bill of materials

**直接下载双语 Excel：[V4-V4pro-BOM.xlsx](hardware/bom/V4-V4pro-BOM.xlsx)**。工作簿分别列出 V4 与 V4-pro，包含中英文名称、数量、单位、规格、商品链接、来源及待确认项。CSV 适合在线查看和版本比较。

**Download the bilingual Excel workbook: [V4-V4pro-BOM.xlsx](hardware/bom/V4-V4pro-BOM.xlsx).** Separate V4 and V4-pro sheets include Chinese and English names, quantities, units, specifications, supplier links, sources, and unresolved items. CSV files support online inspection and version comparison.

[BOM 发布页 / BOM release downloads](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/releases/tag/bilingual-bom-2026-10-22) · [SHA-256 校验值 / Checksums](hardware/bom/SHA256SUMS.txt)

| 型号 / Variant | 在线说明 / Documentation | 表格 / Table |
| --- | --- | --- |
| V4 | [中文 BOM](docs/bom.md) · [English BOM](docs/bom.en.md) | [V4 CSV](hardware/bom/V4-BOM.csv) |
| V4-pro | [中文 BOM](docs/bom.md) · [English BOM](docs/bom.en.md) | [V4-pro CSV](hardware/bom/V4-pro-BOM.csv) |
| 两型号 / Both | [双语数据 / Bilingual data](hardware/bom/bom-data.json) | [Excel 下载 / Download](hardware/bom/V4-V4pro-BOM.xlsx) |

数量未明确的条目保留为空并附说明；舵机数量、继电器电流和 Pro 舵机供电的历史冲突均保留。商品记录用于核对规格，未确认当前价格或在售情况。机械加工、打印件与线束也纳入清单，完整数量仍须结合对应 CAD 和实际装配核对。

Unspecified quantities remain blank with explanatory notes. Historical conflicts in servo count, relay current rating, and Pro servo voltage are retained. Supplier records help identify specifications; current prices and availability have not been verified. The BOM includes machining, printed parts, and wiring, with final quantities to be checked against the matching CAD and assembly.

## 机械与制作 / Structure and assembly

Git 中提供 STEP / DWG 等加工与交换文件；完整 SolidWorks 原包保存在 [机械源文件 Release / Mechanical source release](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/releases/tag/mechanical-source-2026-10-22)。原包包含装配关系，下载需要有仓库访问权限的 GitHub 账户。

STEP / DWG and other manufacturing or exchange files are stored in Git. Complete SolidWorks source archives, including native assemblies, are available in the release linked above. Access requires a GitHub account with permission to this private repository.

| 资料 / Material | 中文 | English |
| --- | --- | --- |
| 机械下载 / Mechanical files | [文件索引](hardware/README.md) | [File index](hardware/README.en.md) |
| V4 制作与接线 / V4 build and wiring | [硬件说明](docs/hardware-v4.md) | [Hardware guide](docs/hardware-v4.en.md) |
| V4-pro 制作与接线 / V4-pro build and wiring | [硬件说明](docs/hardware-v4-pro.md) | [Hardware guide](docs/hardware-v4-pro.en.md) |
| 系统结构 / System architecture | [系统结构](docs/architecture.md) | [Architecture](docs/architecture.en.md) |

## 控制代码 / Controllers

V4 使用用户指定的 IWIN-FINS `V4-ly` 分支；V4-pro 使用 FineSUB `v4pro1` 固定源码，并提供到 `V4pro2` 的差异补丁。导入记录保留提交、依赖、构建入口及限制。

V4 uses the specified IWIN-FINS `V4-ly` branch. V4-pro uses a pinned FineSUB `v4pro1` snapshot with a patch for `V4pro2`. Import records identify commits, dependencies, build entry points, and limitations.

| 代码 / Code | 中文 | English |
| --- | --- | --- |
| [V4 firmware](firmware/v4) | [源码导入](firmware/v4/IMPORT.md) · [协议与构建](docs/firmware-v4.md) | [Import record](firmware/v4/IMPORT.en.md) · [Protocol and build](docs/firmware-v4.en.md) |
| [V4-pro firmware](firmware/v4-pro) | [源码导入](firmware/v4-pro/IMPORT.md) · [Pro1 / Pro2](firmware/v4-pro/variants/README.md) | [Import record](firmware/v4-pro/IMPORT.en.md) · [Pro1 / Pro2](firmware/v4-pro/variants/README.en.md) |

## 上位机与部署 / Topside software and deployment

V4 的 PC 手柄客户端位于 `software/v4-topside`；Pro 的 Jetson 串口转发、视频及部署模板位于 `software/v4-pro-jetson`。先按型号选择链路，再配置网络和串口。V4 的 TCP 客户端与 Pro 的 UDP 协议不兼容。

The V4 PC gamepad client is in `software/v4-topside`. Pro Jetson serial forwarding, video scripts, and deployment templates are in `software/v4-pro-jetson`. Select the vehicle configuration before configuring networking and UART. The V4 TCP client is incompatible with the Pro UDP protocol.

- 使用与部署 / Operation and deployment: [中文](docs/operation.md) · [English](docs/operation.en.md)
- V4 PC 客户端 / PC client: [中文](software/v4-topside/IMPORT.md) · [English](software/v4-topside/IMPORT.en.md)
- V4-pro Jetson: [中文](software/v4-pro-jetson/README.md) · [English](software/v4-pro-jetson/README.en.md)

## 仓库结构 / Repository layout

```text
firmware/v4/                  IWIN-FINS V4-ly controller source
firmware/v4-pro/              FineSUB pro1, pinned dependencies, pro2 patch
software/v4-topside/          V4 PC gamepad and plotting clients
software/v4-pro-jetson/       Pro Jetson control, video, deployment templates
software/legacy-2025-communication/  Historical image-transfer examples
hardware/bom/                Bilingual Excel, CSV and source data
hardware/v4/                 V4 CAD, manipulator, camera mount, wiring PDF
hardware/v4-pro/              V4-pro CAD and thrust calculation script
docs/                        Chinese and English technical guides
tools/                       Offline integrity checks
```

## 验证与待补充内容 / Validation and remaining gaps

当前已检查来源文件、CAD 和附件完整性、Python 静态语法及离线协议。尚未完成交叉编译或实机联调。原资料没有提供当前 V4 树莓派 `joystick_control` 服务端，以及可确认的 Pro Windows 手柄程序源码；这些缺口明确保留。

Source, CAD, and attachment integrity, Python syntax, and offline protocol encoding have been checked. Cross-compilation and hardware integration have not been completed. The supplied sources omit the current V4 Raspberry Pi `joystick_control` server and confirmed source code for the matching Pro Windows gamepad application.

验证记录 / Validation: [中文](docs/validation.md) · [English](docs/validation.en.md)

已知缺口 / Known limitations: [中文](docs/known-limitations.md) · [English](docs/known-limitations.en.md)

## 来源、参考与贡献 / Sources, reference, and contributions

项目资料来自 IWIN-FINS 团队飞书文档和文档引用的上游仓库。仓库展示参考 [FinsROV: An Underwater Camera-Based Multi-Robot Platform](https://github.com/IWIN-FINS/FinsROV-An-Underwater-Camera-Based-Multi-Robot-Platform) 的双语介绍、结构文件、BOM 和制作说明组织方式。该参考项目的性能参数及许可不自动适用于本仓库。

Project material comes from the IWIN-FINS team's Feishu documents and their referenced upstream repositories. The linked FinsROV project informs the bilingual presentation and organization of structure files, BOM, and build instructions. Its performance specifications and licenses do not automatically apply to this repository.

- 来源 / Sources: [中文](docs/sources.md) · [English](docs/sources.en.md)
- 许可与署名 / Licenses and attribution: [中文](THIRD_PARTY_NOTICES.md) · [English](THIRD_PARTY_NOTICES.en.md)
- 贡献 / Contributing: [中文](CONTRIBUTING.md) · [English](CONTRIBUTING.en.md)

新编项目文档提供完整中英文版本；上游源码注释、原始附件、CAD 文件名及图内文字保留原样。

The authored project documentation is available in Chinese and English. Upstream source comments, original attachments, CAD filenames, and text embedded in original figures remain unchanged.
