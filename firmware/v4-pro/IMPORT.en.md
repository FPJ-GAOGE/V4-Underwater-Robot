# V4-pro firmware provenance, build, and instance differences

[中文](IMPORT.md) | English

This directory contains a snapshot of the [FineSUB](https://gitee.com/gan-ju/FineSUB) `v4pro1` branch, pinned to `5a7832648206e522e9b8e0dff108638789f6afdd`. Its upstream commit message is “V4PRO1 粗调pid版” (V4PRO1 with preliminary PID tuning). The other instance branch, `V4pro2`, is pinned to `847ff0309c54016365c303b686369af9d005db41` and preserved through an [exact patch](variants/v4-pro1-to-pro2.patch).

The Feishu V4-pro manufacturing material references FineSUB without specifying a branch. The two explicitly named upstream branches are retained as reviewable references; **this does not establish that any existing vehicle has either commit flashed onto it**. `pro1` and `pro2` are instance differences within V4-pro, not the two models V4 and V4-pro. The V4 IWIN-FINS / V4-ly firmware is in the neighboring directory.

## Imported scope and licenses

- FineSUB's root [LICENSE](LICENSE) is MPL-2.0. Original authorship, copyright headers, and third-party licenses in each BSP are preserved.
- Original source code, all three BSPs, CubeMX `.ioc` files, MDK projects, OpenOCD configuration, and upstream documentation are retained. Control algorithms, PID parameters, communication, wiring definitions, and safety state machines have not been changed.
- No `.idea/`, `.vscode/`, or MDK user debug options `*.uvoptx` are published at any directory level. This includes 5 files under `Devices/PressureSensor/.idea/`, 4 under `Services/Bus/.idea/`, and ETL's `include/etl/.vscode/settings.json`; these 10 IDE files are excluded from the provenance manifest. Submodules have been expanded into actual dependency files, so the upstream `.gitmodules` is omitted and `git submodule update` is unnecessary.
- CMSIS-DSP is pinned to `4ee6da97c239071283d869de7c636d7621280e7c`: `Include/`, `PrivateInclude/`, `Source/`, README, and the Apache-2.0 LICENSE are included.
- ETL is pinned to `a12dbbd91174e895bdd5492826b0226b9668fc5f`: `include/`, README, version information, and the MIT LICENSE are included. The nested VS Code settings above are excluded; the 10 `.bat` source-generation scripts under `generators/` are retained.
- Dependency examples, test data, documentation-generation tools, and Python packages were not copied. The C/C++ dependency directories referenced by the current firmware CMake configuration are included.
- [UPSTREAM.json](UPSTREAM.json) records complete provenance, exclusions, per-file Git blob IDs, SHA-256 hashes, and byte counts. Files were exported directly from Git objects at the pinned commits to avoid local line-ending settings affecting verification. The manifest describes imported original files, not this document or the patch.

## Build entry points

V4-pro uses **RoboMaster A / STM32F427**. The presence of `Robomaster_C` and `MC_Board` in the source tree is not a reason to select those board targets.

1. Install Keil MDK / Arm Compiler 6 and make `armclang.exe`, `armlink.exe`, and `fromelf.exe` available.
2. The retained upstream MDK project is [BSP/Robomaster_A/MDK-ARM/Robomaster_A.uvprojx](BSP/Robomaster_A/MDK-ARM/Robomaster_A.uvprojx). All 59 referenced files were found, but its file list **does not contain `TaskSUB.cpp`**. It must not be treated as a fully configured current V4-pro application project. Synchronize application sources, header-search paths, and dependency settings before using MDK.
3. The CMake entry point is [CMakeLists.txt](CMakeLists.txt), requiring CMake 3.21 or newer and C++17. The upstream configuration directly names Windows `.exe` tools; it cannot be built as an ordinary Linux GCC project without adaptation.

From this directory, in a Windows terminal configured with Arm Compiler 6 and Ninja, the upstream CMake build can be attempted with:

```powershell
cmake -S . -B build -G Ninja
cmake --build build --target Robomaster_A
```

No usable Arm Compiler 6 installation was found during this preparation, so **firmware compilation and flashing have not been verified**. The original CMake post-build step expects `build/bin/Robomaster_A.elf`; if the generator produces a different name, check the linked output against the `fromelf` input. Upstream build definitions are preserved; unverified changes were not introduced into the control firmware.

## Control and communication entry points

| Item | Current source behavior | Location |
| --- | --- | --- |
| Host commands | UART3, 115200, 8N1; `UARTBuffer<3,30>` calls `Streamer::Decode` | `Interface/Examples/TaskSUB.cpp`, `BSP/Robomaster_A/Core/Src/usart.c` |
| IMU | H30, UART7, 921600, 8N1; `UARTBuffer<7,100>` | Same files as above |
| Thrusters | 8 DSHOT channels with bidirectional 3D mode and telemetry enabled | `Devices/ESC_Motors/DSHOT_Motor.hpp` |
| Servos | Pitch: PWM ID 9; yaw: PWM ID 10; 50 Hz. The A-board table maps these to D and C respectively | `TaskSUB.cpp`, `Robomaster_A.h` |
| Lights | PWM IDs 11 and 12; the A-board table maps these to B and A respectively | Same files as above |
| Pressure sensors | I2C2 → TCA9548A `0x70` → channels 0–3; MS5837 addresses `0x76` / `0x77` | `TaskSUB.cpp` |
| Scheduling | `Setup` initializes the pressure array; `Loop` polls pressure; the TIM7 callback runs devices and tasks | `Interface/Interface.cpp` |

`Services/V5Streamer/Streamer.hpp` strictly accepts **28 bytes**:

| Bytes | Meaning |
| --- | --- |
| 0 | Header `0xAA` |
| 1–20 | 5 `float32` values: yawRate, forwardSpeed, leftRightSpeed, downSpeed, upSpeed |
| 21–23 | 3 `uint8` values: startButton, camFillLight, frameFillLight |
| 24–25 | 2 `int8` values: camYaw, camPitch |
| 26–27 | CRC16/Modbus over the first 26 bytes, low byte first |

The STM32 copies floating-point fields directly from little-endian memory. There is no separate length byte and no V4-ly “device address + command ID” format; do not mix the protocols. The upstream comment says “24 bytes,” which conflicts with the actual `length != 28` check; use the code as the reference.

The current decoder accepts exactly one complete frame per callback. No stream-reassembly buffer was added in this preparation. Serial fragmentation, disconnection, loss-of-control protection, and thruster behavior have not been tested on hardware.

## V4-pro1 / V4-pro2 differences

All firmware differences between the two branches are confined to `TaskSUB.cpp`, `IMUBase.hpp`, and `V5_SUB.hpp`. There is also one IDE run-configuration difference, which is not part of the firmware import.

### Logical motors to DSHOT IDs

Positions in this table come from source variable names. Indices are the zero-based `motors[]` indices used by `V5_SUBBuilder`; **they are not the handwritten numbers in photographs**. Check them together with the board-connector/GPIO mapping in `BSP/Robomaster_A/Robomaster_A.h`.

| Array index | Source position name | pro1 DSHOT ID | pro2 DSHOT ID |
| --- | --- | ---: | ---: |
| 0 | LF Lower / left-front lower | 4 | 4 |
| 1 | LF Upper / left-front upper | 1 | 3 |
| 2 | LB Upper / left-back upper | 2 | 1 |
| 3 | LB Lower / left-back lower | 3 | 2 |
| 4 | RB Lower / right-back lower | 5 | 8 |
| 5 | RB Upper / right-back upper | 8 | 7 |
| 6 | RF Upper / right-front upper | 6 | 5 |
| 7 | RF Lower / right-front lower | 7 | 6 |

### Control, attitude, and pressure

| Item | pro1 | pro2 |
| --- | --- | --- |
| IMU mounting-compensation matrix | `diag(-1,-1,1)` | Identity matrix `diag(1,1,1)` |
| Pressure channels used for depth, zero-based | Average of 1 and 3 | 3 only |
| Pressure-initialization sampling channels | 1, 3 | 3 |
| yaw input handling | Integrates the original input multiplied by `0.001 × 0.4` | Sets absolute values `<0.05` to zero, clamps to `[-1,1]`, then integrates after multiplication by `0.001 × 0.8` |
| Vertical outputs, motors `[1,2,5,6]` | `[-U0,+U1,-U2,-U3]` | `[+U0,-U1,+U2,+U3]` |
| Horizontal outputs, motors `[0,3,4,7]` | `[+L0,-L1,-L2,+L3]` | `[+L0,-L1,+L2,-L3]` |
| PID values | Original parameters retained | Only textual changes from `0.00f` to `0.0f`; numeric values are unchanged |

`U` and `L` mean `upperMotorOutPut` and `lowerMotorOutPut` in the source. The pro2 IMU comment still says “180-degree rotation,” but its matrix is the identity. This conflict is recorded as found; no mounting orientation is assumed on behalf of the code.

### Applying the pro2 patch

This directory remains pro1 by default. To create a pro2 working copy, copy the directory to a separate location and run the following inside that copy:

```powershell
git init
git -c core.autocrlf=false apply --check variants/v4-pro1-to-pro2.patch
git -c core.autocrlf=false apply variants/v4-pro1-to-pro2.patch
```

`git init` makes patch paths relative to this independent copy, avoiding ambiguity with the containing aggregate repository. Command-local `core.autocrlf=false` preserves upstream LF line endings without changing personal Git settings. Apply the patch only once. If the three affected files have already been edited, review conflicts individually first. After application, the per-file hashes in the main manifest still describe pro1; the three modified files are identified by `alternate_patched_files` and `alternate_commit`.

Ports, IMU mounting orientation, thrust signs, and pressure channels must be checked together for each instance. Applying only the motor-index table is insufficient.

## Verification performed

- All 1,658 source files were checked against pinned commits using SHA-256 and Git blob IDs, totaling 49,356,433 bytes.
- The patch passed `git apply --check` in an independent temporary Git working directory. After application, all 3 files matched the pinned `V4pro2` commit byte for byte.
- No thrusters were operated, no vehicle was connected or flashed, and no claim was made that these control parameters are suitable for another physical vehicle.
