# 机械与电气文件

两型号的 CAD 分开存放，文件名相同不代表几何或安装接口一致。

| 目录 / 原包 | 版本与内容 |
| --- | --- |
| `v4/cad/` | V4 结构表中 2025-05-23「完善设计」版本的 STEP / DWG，共21个文件 |
| `v4/camera-mount/` | 2025-07-10 单目相机支架，2个SolidWorks零件 |
| `v4/manipulator/` | 设计文档所附机械爪设计，7个零件、1个装配文件；独立于当前控制链路 |
| `v4/electrical-connection.original.pdf` | 原始1页电气设计图，含历史方案标注；差异见对应硬件说明 |
| `v4-pro/cad/` | V4pro原包的STEP/STP、DWG及`thrust.m`，共35个文件 |

## 完整原生 CAD

[从私有 Release 下载完整原包](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/releases/tag/mechanical-source-2026-10-22)：

- `V4-2025-05-23.zip`：83,275,936 字节，原始文件名为 V4.zip。
- `V4pro.zip`：348,312,112 字节。
- `v4-camera-mount-2025-07-10.zip`：275,425 字节，原始文件名为相机支架.zip。
- `v4-manipulator.zip`：1,530,494 字节，原始文件名为机械爪结构.zip。

原包保留完整SolidWorks零件、装配和工程图关系，不用Git中提取的中性格式代替完整装配工程。V4与Pro原包使用GBK文件名编码；若解压后中文乱码，使用支持GBK/CP936的解压工具。Git中提取的路径已经正确解码为中文。

原包、原包内全部文件和Git内提取文件的SHA-256均列在 [assets-manifest.json](assets-manifest.json)，发布原包的简洁校验表见 [archives.sha256](archives.sha256)。`repository_path=null` 表示该文件只保存在完整原包中。原包中的SolidWorks临时锁文件也在原始清单中，未加入中性格式目录。

## 阅读顺序

查看 [型号差异](../docs/variants.md) → [BOM](../docs/bom.md) → [V4装配接线](../docs/hardware-v4.md) 或 [V4-pro装配接线](../docs/hardware-v4-pro.md)。加工前在CAD中核对材料、尺寸、螺纹和实际采购器件；归档完整性检查不等于制造可行性或耐压验收。

机械与照片资料未附独立开放硬件许可，详见 [第三方许可](../THIRD_PARTY_NOTICES.md)。
