# V4 Firmware Import Record

[中文](IMPORT.md) | English

This directory comes from the user-specified [IWIN-FINS / V4-ly](https://gitee.com/superkinkin/IWIN-FINS/tree/V4-ly/), pinned to upstream commit `42e5f16f5b3ad3e23fceb5eb60868340b365dfd5` and archived on 2026-10-22. Tracked files were extracted using `git -c core.autocrlf=false archive`, disabling local newline conversion. The upstream `.git` directory was not copied. Control algorithms, hardware parameters, communication implementation, and source encoding were not changed.

This firmware is part of the V4 RM-C design material. The [Keil project](MDK-ARM/RM_Frame_C.uvprojx) actually targets `STM32F407IGHx`; the directory name does not prove that it can be flashed directly to any RM board. The RM-A/FineSUB firmware for V4-pro is a separate implementation, described in the repository's model comparison. `V40` here does not mean V4-pro.

## Retained and excluded content

The import retains 347 upstream files, including application source, drivers, middleware, startup files, linker scripts, CubeMX configuration, Keil projects, CMake configuration, original notes, and all included license files. Retained files total approximately 14.9 MB. Their computed Git blob identifiers match the original Git objects in the pinned commit, file by file.

The following generated or nonessential build-input files were excluded:

- `build/**`: upstream-tracked CMake caches, Ninja files, compiler-probe results, and local paths.
- `**/*.lst`, `**/*.map`, `**/*.lnp`, `**/*.dep`, `**/*.build_log.htm`: historical compiler intermediates and logs.
- `**/JLinkSettings.ini`: local debugger settings.
- `Drivers/STM32F4xx_HAL_Driver/*.chm`: eight ST binary user manuals totaling approximately 120.5 MB; they remain available from the pinned upstream commit.

Project configuration such as `.sct`, `.uvprojx`, `.uvoptx`, RTE headers, and startup assembly is retained. Code from other local projects was not merged into this directory.

## Licensing and attribution

The upstream root [LICENSE](LICENSE) contains the GNU GPL version 3 text and is retained unchanged. Original author notices, third-party copyright headers including ST/Arm, and subdirectory licenses are preserved. The new documentation does not replace those licenses.

| Component | Original license file |
| --- | --- |
| Upstream project | [GPL v3 text](LICENSE) |
| CMSIS | [Apache License 2.0](Drivers/CMSIS/LICENSE.txt) |
| ST CMSIS device support | [Original license notice](Drivers/CMSIS/Device/ST/STM32F4xx/LICENSE.txt) |
| STM32 HAL | [Original notice, including BSD-3-Clause fallback terms](Drivers/STM32F4xx_HAL_Driver/LICENSE.txt) |
| ST USB Device Library | [Original notice and SLA0044 text](Middlewares/ST/STM32_USB_Device_Library/LICENSE.txt) |

Third-party directories have their own conditions; the complete driver/middleware collection must not be relabeled under a new single license. Before public distribution, review the license combination of the actual components distributed. This import preserves and identifies existing licenses and does not relicense the code.

## Build entry points and verification status

- Keil: [MDK-ARM/RM_Frame_C.uvprojx](MDK-ARM/RM_Frame_C.uvprojx), recording ARM Compiler 6.15 (ARMCLANG) and device pack `Keil.STM32F4xx_DFP.2.14.0`.
- GCC: [CMakeLists.txt](CMakeLists.txt), using `arm-none-eabi-gcc/g++`, C++11, Cortex-M4, and [STM32F407IGHx_FLASH.ld](STM32F407IGHx_FLASH.ld), producing ELF, HEX, and BIN.
- CubeMX: [RM_Frame_C.ioc](RM_Frame_C.ioc). `CubeMX/MDK-ARM` also contains an upstream project snapshot; do not mix it with the primary Keil entry point.
- Every `FilePath` referenced by the primary Keil project was checked and exists in the imported directory.
- Target-firmware compilation, flashing, and physical thruster validation were not performed. Source integrity and file-reference checks do not demonstrate a successful build or working hardware.

See [V4 firmware documentation](../../docs/firmware-v4.en.md) for parameters, serial commands, build steps, and limitations.

## Reading the historical notes

The original [README notes](说明文档Readme.txt) and [serial protocol text](串口通信协议.txt) are historical documents retained for traceability. Actual behavior in the pinned commit includes:

- `Robot_Version=V40`; the enumeration contains V30, V31, V32, V33, and V40, with no V41 or V4-pro.
- USART6 dispatches device-address and command bytes. The historical `ON/OFF/PRO/VEL` strings are not the current thruster entry point.
- Initial thruster pulse width is 1590 μs. Disabling closed-loop control still outputs the initial pulse width rather than disconnecting the output.
- The watchdog object is absent from the device table, and `MX_IWDG_Init()` is commented out. Default automatic stopping or restarting after communication loss must not be claimed.
- The pressure class still compares bare `CA/VA` strings, which are incompatible with the current first-byte address dispatcher. Calibration Flash writes are also commented out.
- The enumeration is not a complete vehicle-profile switch: thruster parameters and servo channels remain fixed to the V40 arrangement.

These differences are documented by this archival work; upstream control behavior was not silently modified.
