# 来源与版本记录

整理日期：**2026-10-22（Asia/Shanghai）**。飞书内容通过项目所有者授权的 SJTU 账户读取；以下链接是资料出处，不保证无飞书权限的读者能访问。阅读、编译所需已取得的源码、图片和工程文件尽量保存在本仓库或其私有 Release 中。

## 用户指定的三份资料

| 文档 | 读取修订号 | 用途 |
| --- | ---: | --- |
| [V4-PRO制作文档汇总（RM-A主控）](https://iwin-fins.feishu.cn/wiki/RfVuwKMnqisR3rkWaFmcW6bOnVh) | 82 | Pro模型、采购、装配、接线和调试流程 |
| [比赛细节与V4设计](https://iwin-fins.feishu.cn/wiki/Ty7WwhJmCihi7qk3Gx8cAL93n8f) | 4295 | V4设计过程、结构表、机械与电气资料、关联程序附件 |
| [V4使用说明](https://iwin-fins.feishu.cn/wiki/HCfmwuAOSiYgJCkpqcic0lPrnY8) | 277 | V4固件与PC仓库、网络、运行方式与历史故障记录 |

## 直接关联资料

| 文档或表格 | 用途 |
| --- | --- |
| [V4接线简化图](https://iwin-fins.feishu.cn/wiki/AW4WwJtehiJW3mk2VVbcxBXOnje) | 内容实际为A板 / NANO / 电调方案，归入Pro接线原图 |
| [ORIN NX配置（ORIN NANO同样适用](https://iwin-fins.feishu.cn/docx/S8I9dBMNmo2dIPxhWzkcyeQsnoP)（修订1312） | Jetson三脚本、UART3、USB相机、视频与自启动 |
| [AUV电路连接设计及元件清单](https://iwin-fins.feishu.cn/wiki/R7XGwUqQZiNfqGkNGA8cCkj1nsh)（修订667） | C板 / PCA9685接线和1页原始PDF |
| [V4配件清单](https://iwin-fins.feishu.cn/wiki/BQjFwY7GeiFW2KkmTXWcBC7fn8d) | BOM、连接器与线长；和旧电路资料的冲突另列 |
| V4设计文档内嵌「结构设计」表（读取修订28） | 6条版本记录；最新完整V4为2025-05-23，另有2025-07-10相机支架 |
| [上位机和树莓派通信](https://iwin-fins.feishu.cn/wiki/YMpxw929tiODo1kBmVRcYpy4noh) | 2025年图像通信示例，非当前joystick_control |
| [树莓派快速串口通信（调试中使用）](https://iwin-fins.feishu.cn/wiki/KDRZwEfYBiq2ERkQRXpccfksnxd) | 历史SSH和串口调试方法，未保留凭据 |
| [树莓派环境配置](https://iwin-fins.feishu.cn/wiki/YRXKw1LsEi6JpskuqOAcU88Pnyc) | 历史环境参考，没有作为已锁定的新部署环境 |

## 固定软件来源

| 本地目录 | 上游与分支 | 固定提交 |
| --- | --- | --- |
| `firmware/v4` | [IWIN-FINS / V4-ly](https://gitee.com/superkinkin/IWIN-FINS/tree/V4-ly/)（用户明确指定） | `42e5f16f5b3ad3e23fceb5eb60868340b365dfd5` |
| `firmware/v4-pro` | [FineSUB / v4pro1](https://gitee.com/gan-ju/FineSUB/tree/v4pro1/) | `5a7832648206e522e9b8e0dff108638789f6afdd` |
| Pro差异补丁 | [FineSUB / V4pro2](https://gitee.com/gan-ju/FineSUB/tree/V4pro2/) | `847ff0309c54016365c303b686369af9d005db41` |
| `software/v4-topside` | [rov-v4-wired-joystick / master](https://gitee.com/superkinkin/rov-v4-wired-joystick) | `a65bc514808d2d8de929f05d2f42f31ed5bc848c` |
| `software/v4-pro-jetson/upstream` | Orin配置文档中的三个附件 | 无Git提交，按SHA-256记录 |
| `software/legacy-2025-communication` | V4设计文档中的wirelesscommunicator.zip | 无Git提交，按原包文件校验 |

FineSUB依赖CMSIS-DSP与ETL的固定提交及文件清单见 [UPSTREAM.json](../firmware/v4-pro/UPSTREAM.json)。V4源码清单见 [UPSTREAM.json](../firmware/v4/UPSTREAM.json)。本仓库保留了构建相关源文件，未导入完整上游Git历史。

## 机械附件取舍

两个完整型号模型、后续相机支架和机械爪原包都保留；提取路径和每个文件的SHA-256见 [机械清单](../hardware/assets-manifest.json)。没有把V3.4或较早的V4模型替代2025-05-23版本。V4-pro原包中的文件名 `V4外形.STEP` 按其所在原包归属于Pro，未改归属。

未收录比赛规则PDF、采购商软件安装包、行程费用、人员任务进展、相机标定照片集等与本次可复用项目主体无直接关系的内容。2025年的控制压缩包与另一维护者仓库属于历史来源，主控制代码优先采用用户明确指定的 `V4-ly`。

文档改写区分「设计计划」「源码实际行为」「已验证结果」。缺失服务端、不明确许可或相互矛盾的硬件参数保留为可追溯的缺口，详见 [known-limitations.md](known-limitations.md)。
