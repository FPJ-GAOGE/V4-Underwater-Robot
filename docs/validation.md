# 本次整理的验证记录

中文 | [English](validation.en.md)

验证日期：2026-10-22。范围为资料、源码快照及离线协议，不包含实机运行。

## 已完成

| 项目 | 检查与证据 |
| --- | --- |
| 三份用户指定飞书文档 | 已通过SJTU账户读取全文；主文档修订号和直接关联资料列于 [sources.md](sources.md) |
| V4控制源码 | 347份导入文件与固定提交逐一核对SHA-256和Git blob ID；[UPSTREAM.json](../firmware/v4/UPSTREAM.json) |
| V4-pro控制源码与依赖 | 1658份来源文件与固定Git对象逐一核对，49,356,433字节；排除10个嵌套IDE文件，保留10个ETL生成脚本；[UPSTREAM.json](../firmware/v4-pro/UPSTREAM.json) |
| pro1→pro2补丁 | 在独立干净副本通过`git apply --check`；应用后3文件逐字节匹配目标提交 |
| V4 PC源码 | 5份原始文件以`git show`核对Git对象，恢复原始行尾；[UPSTREAM.json](../software/v4-topside/UPSTREAM.json) |
| CAD | 4个原始压缩包，共275个非目录条目；66个文件提取到Git，逐个核验SHA-256；中文文件名按原始编码正确解码 |
| PDF与图片 | 原始电气PDF共1页，已渲染查看；接线简图按实际A板内容归入Pro；图中文字与正文电压冲突已标明 |
| Python语法 | software目录12份Python源码静态编译通过；未导入其顶层运行代码 |
| Jetson控制协议 | 仅从AST提取纯定义，使用内存假串口核验41字节手柄输入、25字节有效载荷、28字节串口帧、字段方向和CRC；`123456789` CRC16/Modbus结果为`0x4B37` |
| 源码说明 | 独立源码审阅补充H30依赖并明确DSHOT/PWM差异 |
| 私有性与净化 | GitHub仓库设置为private；原始飞书响应与认证内容在忽略目录中，不提交设备密码、评论、人员/经费记录 |

## 双语文档与 BOM 检查

20 组新编文档均有完整中英文版本及语言导航，已核对技术数字、单位、协议、型号差异和本地文件链接。首页按 FinsROV 参考项目采用双语展示，独立列出 BOM、机械、控制器和部署入口。

双语 [Excel BOM](../hardware/bom/V4-V4pro-BOM.xlsx) 包含 V4 中文 / English、V4-pro 中文 / English 共 4 个工作表，覆盖 95 项记录（V4 56 项、V4-pro 39 项），其中 47 项数量未知，保持空值。已逐项核对 Excel 与两份 CSV 对应的 JSON 来源数据，包括名称、数量、单位、规格、状态和出处；四个工作表均有筛选表格与冻结窗格，66 个商品超链接对应原记录。工作表均已渲染查看，未使用虚构价格或汇总采购金额。

BOM 覆盖已取得的原配件表、制作说明、旧方案和可识别机械条目，未声称完成装配体实例计数。历史替代项、可选设计资料及缺失数量均单独标识，不应将 95 行直接作为一台潜器的采购订单。

## 可以重新运行的离线检查

在仓库根目录使用Python 3.10或更高版本执行：

```sh
python tools/validate_repository.py
```

它会核对源码清单、提取CAD及附件SHA-256、Python语法、中英文档导航与本地链接、BOM的JSON/CSV/Excel逐项对应及超链接，以及内存中的协议编码。程序不会执行原始脚本的串口/socket初始化，不会连接潜器或启动GStreamer。

原包下载后还应对照 [archives.sha256](../hardware/archives.sha256) 检查SHA-256；工作目录的校验通过不能单独证明Release附件传输完整。导入哈希证明来源一致，不证明原程序没有缺陷。

## 未完成、未作声称的验证

- 本机没有可用的固件交叉工具链，未完成V4或Pro的目标编译、链接、烧录与运行。Pro的MDK工程尚未包含TaskSUB.cpp，不能因工程文件存在就称可构建。
- 未安装或联调全部Pygame、PySerial、JetPack、相机和GStreamer环境。
- 未连接潜器，未测试电调中位、DSHOT模式、旋向、闭环稳定性、断联停机、水密、耐压、续航或最大速度。
- 原资料缺失的V4树莓派服务端和Pro Windows客户端未自行替写；许可未明确的材料没有新增许可声明。

具体限制保留在 [known-limitations.md](known-limitations.md)。
