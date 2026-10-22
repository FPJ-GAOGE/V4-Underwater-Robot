# Sources and version record

[中文](sources.md) | English

Preparation date: **2026-10-22 (Asia/Shanghai)**. Feishu content was read through the SJTU account authorized by the project owner. The links below identify sources; readers without Feishu permissions may not be able to access them. Acquired source code, images, and engineering files needed for reading and building are retained in this repository or its private Release where possible.

## Presentation and organization reference

The user-specified reference project is [FinsROV — An Underwater Camera-Based Multi-Robot Platform](https://github.com/IWIN-FINS/FinsROV-An-Underwater-Camera-Based-Multi-Robot-Platform), at `main` commit `1bc38e83d8af669339c7a86f0a14e2180676347f`. Its `Code/`, `Structure/`, and `Making_Instructions/` categories, paragraph-by-paragraph bilingual homepage, and bilingual bills of materials at `Structure/zh-CN/bom表.xlsx` / `Structure/En/BOM.xlsx` inform this project's presentation and organization.

This reference is not evidence for V4 / V4-pro measured performance, hardware configuration, software compatibility, or redistribution rights. Technical facts and rights records for this project remain grounded in the original V4 / V4-pro material and pinned source code listed below.

## Three user-specified documents

| Document | Revision read | Purpose |
| --- | ---: | --- |
| [V4-PRO Manufacturing Documentation (RM-A Controller)](https://iwin-fins.feishu.cn/wiki/RfVuwKMnqisR3rkWaFmcW6bOnVh) | 82 | Pro models, procurement, assembly, wiring, and commissioning |
| [Competition Details and V4 Design](https://iwin-fins.feishu.cn/wiki/Ty7WwhJmCihi7qk3Gx8cAL93n8f) | 4295 | V4 design history, structure table, mechanical/electrical material, and related program attachments |
| [V4 User Guide](https://iwin-fins.feishu.cn/wiki/HCfmwuAOSiYgJCkpqcic0lPrnY8) | 277 | V4 firmware and PC repositories, networking, operation, and historical fault records |

## Directly related material

| Document or table | Purpose |
| --- | --- |
| [Simplified V4 Wiring Diagram](https://iwin-fins.feishu.cn/wiki/AW4WwJtehiJW3mk2VVbcxBXOnje) | Actual content describes the A-board / NANO / ESC arrangement, so it is classified as an original Pro wiring diagram |
| [ORIN NX Setup (also applicable to ORIN NANO)](https://iwin-fins.feishu.cn/docx/S8I9dBMNmo2dIPxhWzkcyeQsnoP), revision 1312 | Three Jetson scripts, UART3, USB camera, video, and startup |
| [AUV Electrical Connections and Component List](https://iwin-fins.feishu.cn/wiki/R7XGwUqQZiNfqGkNGA8cCkj1nsh), revision 667 | C-board / PCA9685 wiring and a one-page original PDF |
| [V4 Parts List](https://iwin-fins.feishu.cn/wiki/BQjFwY7GeiFW2KkmTXWcBC7fn8d) | BOM, connectors, and cable lengths; conflicts with older electrical material are recorded separately |
| “Structural Design” table embedded in the V4 design document, revision 28 read | 6 version records; latest complete V4 set dated 2025-05-23, plus a camera mount dated 2025-07-10 |
| [PC and Raspberry Pi Communication](https://iwin-fins.feishu.cn/wiki/YMpxw929tiODo1kBmVRcYpy4noh) | 2025 image-communication example, not the current joystick_control service |
| [Quick Raspberry Pi Serial Communication for Debugging](https://iwin-fins.feishu.cn/wiki/KDRZwEfYBiq2ERkQRXpccfksnxd) | Historical SSH and serial debugging methods; credentials were not retained |
| [Raspberry Pi Environment Setup](https://iwin-fins.feishu.cn/wiki/YRXKw1LsEi6JpskuqOAcU88Pnyc) | Historical environment reference, not a newly locked deployment environment |

## Pinned software sources

| Local directory | Upstream and branch | Pinned commit |
| --- | --- | --- |
| `firmware/v4` | [IWIN-FINS / V4-ly](https://gitee.com/superkinkin/IWIN-FINS/tree/V4-ly/), explicitly selected by the user | `42e5f16f5b3ad3e23fceb5eb60868340b365dfd5` |
| `firmware/v4-pro` | [FineSUB / v4pro1](https://gitee.com/gan-ju/FineSUB/tree/v4pro1/) | `5a7832648206e522e9b8e0dff108638789f6afdd` |
| Pro difference patch | [FineSUB / V4pro2](https://gitee.com/gan-ju/FineSUB/tree/V4pro2/) | `847ff0309c54016365c303b686369af9d005db41` |
| `software/v4-topside` | [rov-v4-wired-joystick / master](https://gitee.com/superkinkin/rov-v4-wired-joystick) | `a65bc514808d2d8de929f05d2f42f31ed5bc848c` |
| `software/v4-pro-jetson/upstream` | Three attachments in the Orin setup document | No Git commit; recorded by SHA-256 |
| `software/legacy-2025-communication` | wirelesscommunicator.zip in the V4 design document | No Git commit; checked against files in the original archive |

Pinned CMSIS-DSP and ETL dependencies and the FineSUB file manifest are recorded in [UPSTREAM.json](../firmware/v4-pro/UPSTREAM.json). The V4 source manifest is in its own [UPSTREAM.json](../firmware/v4/UPSTREAM.json). Build-related sources are retained; full upstream Git histories were not imported.

## Mechanical attachment selection

Original archives for both complete vehicle models, the later camera mount, and the gripper are retained. Extraction paths and per-file SHA-256 hashes are in the [mechanical manifest](../hardware/assets-manifest.json). V3.4 or earlier V4 models were not substituted for the 2025-05-23 version. The file `V4外形.STEP` inside the V4-pro archive remains classified as Pro according to its containing original archive; it was not reassigned based on its filename.

Competition-rule PDFs, vendor software installers, travel/expense records, personnel progress notes, camera-calibration image collections, and other content not directly relevant to the reusable project were excluded. The 2025 control archive and another maintainer's repository are historical sources; the primary control code follows the user's explicit selection of `V4-ly`.

The rewritten documentation distinguishes design plans, actual source behavior, and verified results. Missing servers, unclear licenses, and conflicting hardware parameters remain traceable gaps; see [known-limitations.en.md](known-limitations.en.md).
