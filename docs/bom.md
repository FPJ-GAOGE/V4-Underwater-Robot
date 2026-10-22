[中文](bom.md) | [English](bom.en.md)

# V4 / V4-pro 物料清单（BOM）

本页与可下载表格使用同一份[结构化双语数据](../hardware/bom/bom-data.json)。整理日期：**2026-10-22**。共 **95 条记录：V4 56 条，V4-pro 39 条**；其中包括历史替代项、可选机构和数量待确认项，不能将全部行直接作为单台采购订单。

## 下载

- [Excel：V4 / V4-pro 双语BOM](../hardware/bom/V4-V4pro-BOM.xlsx)
- [CSV：V4](../hardware/bom/V4-BOM.csv) · [CSV：V4-pro](../hardware/bom/V4-pro-BOM.csv)
- [JSON：双语数据、出处和状态](../hardware/bom/bom-data.json)
- [独立下载发布页](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/releases/tag/bilingual-bom-2026-10-22) · [SHA-256 校验值](../hardware/bom/SHA256SUMS.txt)

## 如何使用数量和状态

- **待确认**表示原文没有可确认的数量，JSON中为`null`，不是0。数量列记录来源数值，不声称已完成装配体的实例数量核算。
- “件/组/套”为整理用计数单位；原文未给商品包装单位时，已在数量备注中指出。零浮力网线的100米是长度记录，不是100条电缆。
- **历史替代项**不能与当前清单累加；**可选设计资料**不代表每台默认安装。图纸存在某零件也不能推断所需件数。
- **来源冲突**必须先按实际潜器核对：V4舵机2/4、继电器30A/120A、两代接线端子布局；Pro舵机供电5V/8.4V和端子数量。
- 商品链接只保留原记录中的商品ID，已去除账号/跟踪参数；未检查当前价格、在售情况或商品选项。相同链接可能有带/不带电调、不同电压或量程等选项，不能只按链接下单。
- V4和V4-pro的推进器线束、主控、舱内计算机、控制信号和CAD分别配套；不能用同名零件或相同商品链接证明两者通用。

V4原配件表26条均已纳入；旧电气资料的补充项单列状态，H30仅保留一条源码依赖记录。Pro连接器虽随设备给出定制要求，但未单列采购数量，因此没有擅自填成推进器/传感器的数量。未给出材料、公差、密封等级或承压性能的结构件，仍以工程图和实机确认记录为准。

## V4

### 电子与执行器

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-thruster`<br>推进器 | 8 件 | 原V4为电源正/负/PWM三线；线长按安装位置实测定制，不采用Pro三相线接法。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical)<br>[原商品链接](https://item.taobao.com/item.htm?id=591864716045) |
| `v4-servo`<br>舵机 | 2 件<br>配件表数量2；旧电路表数量4，不相加。 | 旧电路表和V4-ly固件保留4路；按当前相机/机构实装数量确认。 | 来源冲突，待实物确认<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical) · [v4-firmware](#source-v4-firmware)<br>[原商品链接](https://item.taobao.com/item.htm?id=853148098562) |
| `v4-pressure-sensor`<br>水深/水压传感器 | 4 件 | 配件表：不锈钢、10m选项；旧商品标题含MS5837-30BA。选项/实际型号须核对，10m不是整机潜深。 旧接线使用随附转接板，需核对是否包含在传感器供货中。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical)<br>[原商品链接](https://item.taobao.com/item.htm?id=605375059849) |
| `v4-battery`<br>4S电池 | 1 件 | XT60主电源接口；旧电路文档记10000mAh、25C，配件表只确认4S/XT60。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical)<br>[原商品链接](https://item.taobao.com/item.htm?id=609820541557) |
| `v4-relay`<br>继电器/电源开关模块 | 1 件 | 配件表写30A，旧电路文档写120A；原短链接不作为型号或额定电流证明。 | 来源冲突，待实物确认<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical)<br>无已核实的商品URL |
| `v4-rotary-switch`<br>水下旋钮开关 | 1 件 | 主电源控制路径中的开关；按实际器件核对接口和触点能力。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical)<br>[原商品链接](https://item.taobao.com/item.htm?id=746272975983) |
| `v4-buck-converter`<br>降压模块 | 1 件 | 用于树莓派和机构支路；各负载电压/电流分别核对。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical)<br>[原商品链接](https://item.taobao.com/item.htm?id=856012812115) |
| `v4-power-distribution`<br>分电板 | 1 件<br>数量1来自旧电路表；当前配件表未列。 | 旧电路用于树莓派/舵机支路；正文也允许自制分线，不与其他分支方案重复采购。 | 历史替代项，不与当前清单累加<br>[v4-electrical](#source-v4-electrical)<br>[原商品链接](https://item.taobao.com/item.htm?id=814381038913) |
| `v4-raspberry-pi`<br>树莓派4B | 1 件 | 舱内计算机；内存规格和存储介质未在配件表明确。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical)<br>[原商品链接](https://detail.tmall.com/item.htm?id=705995475782) |
| `v4-controller`<br>RoboMaster C板 | 1 件 | 系统资料中的V4主控；固件工程目标为STM32F407IGHx，按实际板卡核对。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical) · [v4-firmware](#source-v4-firmware)<br>[原商品链接](https://item.taobao.com/item.htm?id=611226266955) |
| `v4-h30-imu`<br>H30 IMU（源码依赖） | 待确认 件<br>原文未给出可确认的数量；不等于0。 | V4-ly姿态反馈来自H30，USART1 921600 8N1；启动还检查BMI088。实际H30采购型号与安装需补录。 | 数量或规格缺失<br>[v4-firmware](#source-v4-firmware)<br>无已核实的商品URL |
| `v4-pca9685`<br>PCA9685 PWM扩展板 | 1 件 | 推进器使用通道0–7；固件舵机预留通道8–11，逻辑VCC与V+负载电源区分。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical) · [v4-firmware](#source-v4-firmware)<br>[原商品链接](https://detail.tmall.com/item.htm?id=624960039615) |
| `v4-i2c-expander`<br>I2C扩展板 | 1 件 | 固件为TCA9548A；0–3通道接压力传感器，4通道接PCA9685。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical) · [v4-firmware](#source-v4-firmware)<br>[原商品链接](https://item.taobao.com/item.htm?id=612860784973) |
| `v4-camera`<br>相机 | 1 件 | 完整设计含双目方案，后有2025-07-10单目支架更新；按所选支架与接口确认。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-design](#source-v4-design)<br>[原商品链接](https://item.taobao.com/item.htm?id=793615736142) |
| `v4-wireless-debugger`<br>无线调试/烧写器 | 1 件 | 按实际SWD接口与工具支持情况核对。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>[原商品链接](https://item.taobao.com/item.htm?id=745548099223) |

### 连接器与线材

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-servo-thruster-connectors`<br>舵机/推进器水密接插件 | 12 组<br>原表记12，未写包装单位；按配对说明暂列组，须核对。旧8推进器+4舵机与当前舵机2的差异勿忽略。 | 3芯母座+公缆；先确定设备位置和线长，再交由厂家定制。 | 来源冲突，待实物确认<br>[v4-parts](#source-v4-parts)<br>[原商品链接](https://item.taobao.com/item.htm?id=811948973422) |
| `v4-connector-caps`<br>3芯公堵头 | 2 件 | 用于未使用的舵机/推进器接口，与母座配套。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>[原商品链接](https://item.taobao.com/item.htm?id=811948973422) |
| `v4-pressure-sleeves`<br>水压计外置套管 | 4 件 | 用于传感器线缆延长和灌封；密封工艺按所购材料说明。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-design](#source-v4-design)<br>[原商品链接](https://item.taobao.com/item.htm?id=781752613399) |
| `v4-pressure-connectors`<br>水压计水密接插件 | 4 组<br>原表数量4；配对件以组列出，商品包装单位待确认。 | 定制00W 4芯插头+插座；厂家4芯电缆65cm。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>[原商品链接](https://item.taobao.com/item.htm?id=619481441943) |
| `v4-ethernet-tether`<br>零浮力网线 | 100 米<br>100为原表长度记录，不是100条电缆；是否对应单台需求待确认。 | 原表数量栏写“零浮力网线，100m”；端接、外径、芯数与长度定义需和接插件确认。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>无已核实的商品URL |
| `v4-tether-connector`<br>电缆接插件 | 1 件 | 1W 8芯插头；配对插座与端接方案未完整列出。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>无已核实的商品URL |
| `v4-camera-connector`<br>相机接插件 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 1W 4芯插头；按实际相机和穿舱方案核对。 | 数量或规格缺失<br>[v4-parts](#source-v4-parts)<br>无已核实的商品URL |
| `v4-terminal-20p`<br>H3800 20P接线端子 | 2 件 | 按当前板架孔位与线束表配套。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>[原商品链接](https://item.taobao.com/item.htm?id=661948958655) |
| `v4-terminal-16p`<br>H3803 16P接线端子 | 2 件 | 按动力/支路线束和实物端子规格核对。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>[原商品链接](https://item.taobao.com/item.htm?id=551775262138) |
| `v4-terminal-6p`<br>H3800 6P接线端子 | 2 件 | 旧电路正文另有6针/7针布局；不与历史端子行累加。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts) · [v4-electrical](#source-v4-electrical)<br>[原商品链接](https://item.taobao.com/item.htm?id=661948958655) |
| `v4-bridge-3-5mm`<br>3.5mm 10P短接件 | 2 件 | FBS10-3.5，红色，黄铜；与端子系列匹配。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>[原商品链接](https://detail.tmall.com/item.htm?id=622709470719) |
| `v4-bridge-6-2mm`<br>6.2mm 10P短接件 | 2 件 | 10P EB连接件；与端子系列匹配。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>[原商品链接](https://item.taobao.com/item.htm?id=551775262138) |
| `v4-usb-c-power-cable`<br>树莓派Type-C供电线 | 1 件 | 原表记录双左右弯头；按供电电流和安装空间核对。 | 原文列明，采购前核对<br>[v4-parts](#source-v4-parts)<br>[原商品链接](https://detail.tmall.com/item.htm?id=739417974339) |

### 结构与加工件

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-side-support`<br>侧支撑 | 待确认 件<br>CAD存在该零件，未从装配体确认实例数；不套用Pro数量。 | 主体/侧支撑.STEP；同时提供DWG。 V4铝合金加工说明为黑色阳极氧化、哑光；具体材料及公差以工程图为准。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-bottom-support`<br>底支撑2 | 待确认 件<br>CAD存在该零件，未从装配体确认实例数；不套用Pro数量。 | 主体/底支撑2.STEP；同时提供DWG。 V4铝合金加工说明为黑色阳极氧化、哑光；具体材料及公差以工程图为准。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-upper-thruster-support`<br>上推进器支撑 | 待确认 件<br>CAD存在该零件，未从装配体确认实例数；不套用Pro数量。 | 主体/上推进器支撑.STEP；同时提供DWG。 V4铝合金加工说明为黑色阳极氧化、哑光；具体材料及公差以工程图为准。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-corner-piece`<br>角件 | 待确认 件<br>CAD存在该零件，未从装配体确认实例数；不套用Pro数量。 | 主体/角件.STEP。 V4铝合金加工说明为黑色阳极氧化、哑光；具体材料及公差以工程图为准。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-rear-endcap`<br>后舱盖165 | 待确认 件<br>CAD存在该零件，未从装配体确认实例数；不套用Pro数量。 | 密封舱/后舱盖165.STEP；同时提供DWG。 V4铝合金加工说明为黑色阳极氧化、哑光；具体材料及公差以工程图为准。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-gimbal-base`<br>云台底架 | 待确认 件<br>CAD存在该零件，未从装配体确认实例数；不套用Pro数量。 | 相机云台/云台底架.STEP；同时提供DWG。 V4铝合金加工说明为黑色阳极氧化、哑光；具体材料及公差以工程图为准。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-rear-gimbal-base`<br>后云台底架 | 待确认 件<br>CAD存在该零件，未从装配体确认实例数；不套用Pro数量。 | 相机云台/后云台底架.STEP；同时提供DWG。 V4铝合金加工说明为黑色阳极氧化、哑光；具体材料及公差以工程图为准。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-sensor-sleeve-mount`<br>外置套管固定加工件 | 待确认 件<br>CAD存在该零件，未从装配体确认实例数；不套用Pro数量。 | 水深传感器/外置套管固定加工版.STEP。 V4铝合金加工说明为黑色阳极氧化、哑光；具体材料及公差以工程图为准。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-sealed-housing`<br>密封舱及配套密封/固定件 | 待确认 组（待展开）<br>原文未给出可确认的数量；不等于0。 | 制造步骤提及密封舱与后舱盖；未给出完整舱体/密封件采购展开和额定潜深。 | 数量或规格缺失<br>[v4-design](#source-v4-design)<br>无已核实的商品URL |

### 打印件与内部支撑

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-upper-electronics-tray`<br>上层架板 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 对应CAD：密封舱/舱内/上层架板.STEP；加工方式、材料和数量按所选装配确认，不能仅凭文件存在推断。 | 数量或规格缺失<br>[v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-lower-electronics-tray`<br>下层架板 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 对应CAD：密封舱/舱内/下层架板.STEP；加工方式、材料和数量按所选装配确认，不能仅凭文件存在推断。 | 数量或规格缺失<br>[v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-battery-tray`<br>电池架 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 对应CAD：密封舱/舱内/电池架.STEP；加工方式、材料和数量按所选装配确认，不能仅凭文件存在推断。 | 数量或规格缺失<br>[v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-camera-link`<br>相机连接件 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 对应CAD：相机云台/相机连接件.STEP；加工方式、材料和数量按所选装配确认，不能仅凭文件存在推断。 | 数量或规格缺失<br>[v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-servo-link`<br>舵机连接件 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 对应CAD：相机云台/舵机连接件.STEP；加工方式、材料和数量按所选装配确认，不能仅凭文件存在推断。 | 数量或规格缺失<br>[v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-buoyancy-shell`<br>浮力外壳2 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 对应CAD：浮力外壳/浮力外壳2.STEP；加工方式、材料和数量按所选装配确认，不能仅凭文件存在推断。 | 数量或规格缺失<br>[v4-cad](#source-v4-cad)<br>无已核实的商品URL |

### 紧固件与装配材料

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-switch-extension-wire`<br>24AWG开关延长线 | 待确认 米<br>原文未给出可确认的数量；不等于0。 | 旧电路只在需要延长旋钮开关线时指定24AWG，未给总长度。 | 数量或规格缺失<br>[v4-electrical](#source-v4-electrical)<br>无已核实的商品URL |
| `v4-power-y-harness`<br>XT60与电源分支线束 | 待确认 组<br>原文未给出可确认的数量；不等于0。 | 旧接线描述Y形分支与XT60连接；使用完成并核验的线束，不将旧改线操作当产品规格。 | 数量或规格缺失<br>[v4-electrical](#source-v4-electrical)<br>无已核实的商品URL |
| `v4-heat-set-inserts`<br>M2.5/M3预埋螺母 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 树莓派、PWM扩展板相关位置用M2.5，其他指定位置M3；并非每个孔都装。 | 数量或规格缺失<br>[v4-design](#source-v4-design)<br>无已核实的商品URL |
| `v4-nylon-standoffs`<br>M2.5/M3尼龙柱 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 树莓派/PWM板M2.5，其余指定位置M3；C板和端子不使用同样支柱。 | 数量或规格缺失<br>[v4-design](#source-v4-design)<br>无已核实的商品URL |
| `v4-fasteners`<br>框架与板架紧固件 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 按2025-05-23装配与螺纹工程图展开，原文无统一规格/数量表。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-potting-material`<br>环氧灌封与绝缘材料 | 待确认 用量待定<br>原文未给出可确认的数量；不等于0。 | 原文要求焊后环氧灌封，旧电气文档要求封住未使用分电点；配比/用量未给。 | 数量或规格缺失<br>[v4-design](#source-v4-design) · [v4-electrical](#source-v4-electrical)<br>无已核实的商品URL |

### 历史接线替代项

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-usb-ttl`<br>USB转TTL模块 | 1 件<br>数量1来自旧电路表；当前配件表未列。 | 旧电路的可选UART连接方法；与直接GPIO UART方案区分。 | 历史替代项，不与当前清单累加<br>[v4-electrical](#source-v4-electrical)<br>无已核实的商品URL |
| `v4-historical-6pin-large`<br>旧方案大6针座 | 4 件 | 旧电路用于推进器，每路为正/负/信号三线。 | 历史替代项，不与当前清单累加<br>[v4-electrical](#source-v4-electrical)<br>无已核实的商品URL |
| `v4-historical-6pin-small`<br>旧方案小6针座 | 2 件 | 旧电路用于舵机；与当前H3800等布局不叠加。 | 历史替代项，不与当前清单累加<br>[v4-electrical](#source-v4-electrical)<br>无已核实的商品URL |
| `v4-historical-7pin-small`<br>旧方案小7针座 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | UART三线+SWD四线；原正文没有明确数量。 | 历史替代项，不与当前清单累加<br>[v4-electrical](#source-v4-electrical)<br>无已核实的商品URL |
| `v4-historical-stlink`<br>ST-Link调试器（旧接法） | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 旧接线步骤提及ST-Link；与无线调试器属不同调试方案，不默认同时采购。 | 历史替代项，不与当前清单累加<br>[v4-electrical](#source-v4-electrical)<br>无已核实的商品URL |

### 可选机构与后续更新

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-monocular-mount-update`<br>2025-07-10单目相机支架更新 | 待确认 组（待展开）<br>原文未给出可确认的数量；不等于0。 | 独立的相机支架1/2两份SLDPRT，不代表每台各采购1件；按所选机构确认。 | 可选设计资料，非默认装配<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |
| `v4-gripper-design`<br>机械爪设计附加包 | 待确认 组（待展开）<br>原文未给出可确认的数量；不等于0。 | 含主动/从动齿轮、圆法兰、夹手、拉杆、基座和顶盖模型；实例数及与主机集成未确认。 | 可选设计资料，非默认装配<br>[v4-design](#source-v4-design) · [v4-cad](#source-v4-cad)<br>无已核实的商品URL |

## V4-pro

### 电子与执行器

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-pro-thruster`<br>无刷推进器（不带电调） | 8 件 | 4正桨+4反桨；一正一反为一组，4组线长65/50/38/32cm。三根电机相线，不是V4正/负/PWM线。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making)<br>[原商品链接](https://item.taobao.com/item.htm?id=591864716045) |
| `v4-pro-esc`<br>四合一电调 | 2 件 | 共8电机通道；FineSUB使用DSHOT双向3D及遥测，电调型号/电压/固件参数待确认。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-firmware](#source-pro-firmware)<br>无已核实的商品URL |
| `v4-pro-pressure-sensor`<br>水深/水压传感器 | 4 件 | 316L不锈钢、10m选项；定制70cm防水线，1红/2黑/3绿SCL/4白SDA。10m不是整机潜深。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making)<br>[原商品链接](https://item.taobao.com/item.htm?id=605375059849) |
| `v4-pro-fill-light`<br>PWM补光灯 | 2 件 | ArduSub脉宽控制探照灯，100°光束角；60cm防水线，1红/2黑/3黄PWM；动力供电按器件核实。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-wiring](#source-pro-wiring)<br>[原商品链接](https://item.taobao.com/item.htm?id=934701227487) |
| `v4-pro-servo`<br>舵机 | 2 件 | 75cm防水线，1红/2黑/3黄PWM。接线简图标5V，制作文字要求A板可调负载电源8.4V，必须核对。 | 来源冲突，待实物确认<br>[pro-making](#source-pro-making) · [pro-wiring](#source-pro-wiring)<br>[原商品链接](https://item.taobao.com/item.htm?id=853148098562) |
| `v4-pro-controller`<br>RoboMaster A板 | 1 件 | STM32F427；推进器为DSHOT，灯具/舵机是独立PWM输出；核对板卡和负载供电。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-firmware](#source-pro-firmware)<br>[原商品链接](https://item.taobao.com/item.htm?id=605165393036) |
| `v4-pro-jetson`<br>Jetson Orin模块与载板 | 1 套<br>原文“NANO×1”；暂以一套计算机记数，模块/载板的独立采购数量待确认。 | 制作清单记“NANO”；关联说明覆盖Orin NX/Nano、达妙载板。不是旧款Jetson Nano的通用替代。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [orin-setup](#source-orin-setup)<br>无已核实的商品URL |
| `v4-pro-battery`<br>定制电池 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 原接线图主电源标24V；容量、电芯串并联、保护及充电接口未明确，不能用商品标题补齐。 | 数量或规格缺失<br>[pro-making](#source-pro-making) · [pro-wiring](#source-pro-wiring)<br>无已核实的商品URL |
| `v4-pro-power-distribution`<br>分电板 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 匹配电池、电调、主控、Jetson支路；具体型号/电流和数量未列。 | 数量或规格缺失<br>[pro-making](#source-pro-making) · [pro-wiring](#source-pro-wiring)<br>无已核实的商品URL |
| `v4-pro-i2c-expander`<br>I2C扩展板 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 制作要求3.3V；当前固件为I2C2→TCA9548A地址0x70→4路水压计。 | 数量或规格缺失<br>[pro-making](#source-pro-making) · [pro-firmware](#source-pro-firmware)<br>[原商品链接](https://item.taobao.com/item.htm?id=677595693967) |
| `v4-pro-imu`<br>IMU | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 供应商刷400Hz固件，配置921600波特率；源码为H30/UART7，确认实际硬件型号。 | 数量或规格缺失<br>[pro-making](#source-pro-making) · [pro-firmware](#source-pro-firmware)<br>[原商品链接](https://item.taobao.com/item.htm?id=649300089333) |
| `v4-pro-wireless-programmer`<br>无线烧写器 | 2 件 | 按各自用途和SWD接口配套，不代表同时连接两台到一块板。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making)<br>[原商品链接](https://item.taobao.com/item.htm?id=745548099223) |
| `v4-pro-powerline-modem`<br>电力载波模块 | 2 件 | PC与潜器通信的成对模块，具体型号/电源参数按实物核对。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making)<br>[原商品链接](https://item.taobao.com/item.htm?id=674728822237) |
| `v4-pro-camera`<br>USB相机 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 制作步骤与Orin说明提及USB相机；型号、数量及实际视频模式未完整给出。 | 数量或规格缺失<br>[pro-making](#source-pro-making) · [orin-setup](#source-orin-setup)<br>无已核实的商品URL |
| `v4-pro-rotary-switch`<br>旋钮开关 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 内部焊线步骤提及；触点额定值和数量未列，不能套用V4开关条目。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |

### 连接器与线材

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-pro-terminal-12p`<br>H3800 12P接线端子 | 2 件 | 文字列12P×2；接线图另外显示20P及舵机12P等，须按最终线束重新确认。 | 来源冲突，待实物确认<br>[pro-making](#source-pro-making) · [pro-wiring](#source-pro-wiring)<br>[原商品链接](https://item.taobao.com/item.htm?id=661948958655) |
| `v4-pro-terminal-drawing-extras`<br>接线图额外20P/12P端子（待合并） | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 用于记录图文差异，不作为在12P×2之外已确认需要追加的数量。 | 来源冲突，待实物确认<br>[pro-wiring](#source-pro-wiring)<br>无已核实的商品URL |
| `v4-pro-thruster-connectors`<br>推进器1W 3芯水密接插件 | 待确认 组（待核对）<br>原文随8个推进器说明定制接插件，未另列采购数量；不能假定计数单位。 | 三相线插头；对应插座要求15A、20cm舱内线；各路配对和包装单位需展开。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |
| `v4-pro-pressure-connectors`<br>水压计00W 4芯水密接插件 | 待确认 组（待核对）<br>与4个水压计配套，原文未单列接插件采购数量。 | 插头1红/2黑/3绿SCL/4白SDA；插座20cm舱内线，传感器侧70cm。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |
| `v4-pro-light-connectors`<br>补光灯1W 3芯水密接插件 | 待确认 组（待核对）<br>与2个灯配套，原文未单列接插件采购数量。 | 1红/2黑/3黄PWM；插座3A、20cm舱内线，设备侧60cm。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |
| `v4-pro-servo-connectors`<br>舵机1W 3芯水密接插件 | 待确认 组（待核对）<br>与2个舵机配套，原文未单列接插件采购数量。 | 1红/2黑/3黄PWM；插座2A、20cm舱内线，设备侧75cm。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |
| `v4-pro-uart-harness`<br>Jetson–RM-A UART连接线 | 待确认 条<br>原文未给出可确认的数量；不等于0。 | GH1.25 4pin线去除5V，仅GND共地、TX↔RX；/dev/ttyTHS1对接UART3，按具体载板核对。 | 数量或规格缺失<br>[orin-setup](#source-orin-setup)<br>无已核实的商品URL |
| `v4-pro-camera-penetrator`<br>相机穿舱件 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 内部装配要求制作相机穿线螺丝；按相机电缆外径和密封方案核对。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |
| `v4-pro-ethernet-cable`<br>以太网/载波连接线 | 待确认 条<br>原文未给出可确认的数量；不等于0。 | Orin与PC调试先直连网线，整机另有载波模块；线长与端接未给统一规格。 | 数量或规格缺失<br>[pro-making](#source-pro-making) · [orin-setup](#source-orin-setup)<br>无已核实的商品URL |

### 结构与加工件

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-pro-side-support`<br>侧支撑 | 2 件 | 主体/侧支撑.STEP与DWG。 材料、公差与表面处理按对应工程图。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |
| `v4-pro-bottom-support`<br>底支撑2 | 2 件 | 原清单称底支撑，文件名底支撑2。 材料、公差与表面处理按对应工程图。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |
| `v4-pro-upper-thruster-support`<br>上推进器支撑 | 2 件 | 主体/上推进器支撑.STEP与DWG。 材料、公差与表面处理按对应工程图。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |
| `v4-pro-front-gimbal-base`<br>前云台底座 | 1 件 | CAD对应相机云台/前云台底架。 材料、公差与表面处理按对应工程图。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |
| `v4-pro-rear-endcap`<br>后舱盖 | 1 件 | CAD对应密封舱/后舱盖165；连接器孔与螺纹按工程图。 材料、公差与表面处理按对应工程图。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |
| `v4-pro-acrylic-tube`<br>亚克力管 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 原采购文列亚克力管及附属固定件，未给数量/尺寸/耐压等级。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |
| `v4-pro-housing-fixings`<br>舱体密封圈/法兰/固定件 | 待确认 组（待展开）<br>原文未给出可确认的数量；不等于0。 | 按完整装配体和配套舱体展开；原文未给完整独立零件数量。 | 数量或规格缺失<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |

### 打印件与内部支撑

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-pro-upper-tray`<br>上层板架 | 1 件 | 按V4pro模型3D打印；材料、层高、填充率未给统一参数。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |
| `v4-pro-lower-tray`<br>下层板架 | 1 件 | 按V4pro模型3D打印；材料、层高、填充率未给统一参数。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |
| `v4-pro-pressure-mount`<br>水压计固定件 | 4 件 | 按V4pro模型3D打印；材料、层高、填充率未给统一参数。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |
| `v4-pro-light-mount`<br>补光灯固定件 | 2 件 | 按V4pro模型3D打印；材料、层高、填充率未给统一参数。 | 原文列明，采购前核对<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |

### 紧固件与装配材料

| ID / 元件 | 数量 / 单位及备注 | 规格与定制要求 | 状态 / 出处 / 原商品记录 |
| --- | --- | --- | --- |
| `v4-pro-heat-set-inserts`<br>M3/M4预埋螺母 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 按上下板架模型区分M3/M4孔位；不能使用V4的M2.5/M3说明代替。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |
| `v4-pro-fasteners`<br>框架与板架螺钉 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 侧支撑要求平头螺丝；具体螺纹、长度和数量按装配体/工程图展开。 | 数量或规格缺失<br>[pro-making](#source-pro-making) · [pro-cad](#source-pro-cad)<br>无已核实的商品URL |
| `v4-pro-esc-capacitors`<br>电调配套电容 | 待确认 件<br>原文未给出可确认的数量；不等于0。 | 内部焊接步骤明确电调焊线与电容，未给电容规格和数量。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |
| `v4-pro-power-harness`<br>动力/分电/开关线束 | 待确认 组（待展开）<br>原文未给出可确认的数量；不等于0。 | 用于电调、分电板、定制电池、旋钮开关和载波模块；线规、保护与长度须按实际负载确定。 | 数量或规格缺失<br>[pro-making](#source-pro-making)<br>无已核实的商品URL |

## 来源

| 来源ID | 资料 |
| --- | --- |
| <a id="source-v4-parts"></a>`v4-parts` | [V4配件清单](https://iwin-fins.feishu.cn/wiki/BQjFwY7GeiFW2KkmTXWcBC7fn8d) |
| <a id="source-v4-electrical"></a>`v4-electrical` | [AUV电路连接设计及元件清单](https://iwin-fins.feishu.cn/wiki/R7XGwUqQZiNfqGkNGA8cCkj1nsh) |
| <a id="source-v4-design"></a>`v4-design` | [比赛细节与V4设计（含结构设计表）](https://iwin-fins.feishu.cn/wiki/Ty7WwhJmCihi7qk3Gx8cAL93n8f) |
| <a id="source-v4-cad"></a>`v4-cad` | [V4 2025-05-23中性CAD及后续可选机构资料](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/tree/main/hardware/v4) |
| <a id="source-v4-firmware"></a>`v4-firmware` | [IWIN-FINS V4-ly固定提交42e5f16](https://gitee.com/superkinkin/IWIN-FINS/tree/42e5f16f5b3ad3e23fceb5eb60868340b365dfd5/) |
| <a id="source-pro-making"></a>`pro-making` | [V4-PRO制作文档汇总（RM-A主控）](https://iwin-fins.feishu.cn/wiki/RfVuwKMnqisR3rkWaFmcW6bOnVh) |
| <a id="source-pro-wiring"></a>`pro-wiring` | [V4接线简化图（内容为V4-pro）](https://iwin-fins.feishu.cn/wiki/AW4WwJtehiJW3mk2VVbcxBXOnje) |
| <a id="source-orin-setup"></a>`orin-setup` | [ORIN NX配置（ORIN NANO同样适用）](https://iwin-fins.feishu.cn/docx/S8I9dBMNmo2dIPxhWzkcyeQsnoP) |
| <a id="source-pro-firmware"></a>`pro-firmware` | [FineSUB v4pro1固定提交5a78326](https://gitee.com/gan-ju/FineSUB/tree/5a7832648206e522e9b8e0dff108638789f6afdd/) |
| <a id="source-pro-cad"></a>`pro-cad` | [V4pro.zip提取的CAD资料](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/tree/main/hardware/v4-pro) |

飞书链接可能需要项目访问权限；CAD和固定源码已在本私有仓库及其Release中保存。参考开源项目仅用于组织方式，不作为本BOM的技术规格或数量来源。

装配细节见[V4硬件](hardware-v4.md)、[V4-pro硬件](hardware-v4-pro.md)和[型号差异](variants.md)。更新物料时先修订双语JSON中的同一条ID，再同步Excel、CSV及两个语言页面，保留冲突依据。
