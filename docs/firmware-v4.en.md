# V4 Control Firmware: Build, Configuration, and Communication

[中文](firmware-v4.md) | English

This guide applies to IWIN-FINS `V4-ly` at the pinned commit `42e5f16f5b3ad3e23fceb5eb60868340b365dfd5` in [firmware/v4](../firmware/v4/IMPORT.en.md). Source files match the upstream specified by the user, excluding only the documented generated artifacts and vendor binary manuals.

The Feishu V4 system documentation describes RM-C, Raspberry Pi, and PCA9685. This branch explicitly targets `STM32F407IGHx`. These are separate facts from system documentation and project configuration, not proof of compatibility with an arbitrary board. The RM-A, Jetson, and FineSUB implementation for V4-pro is separate; changing `Robot_Version` here does not switch to it.

## Build entry points

### Keil MDK

1. Install Keil MDK with the project's recorded ARM Compiler 6.15 (ARMCLANG) and `Keil.STM32F4xx_DFP.2.14.0` device pack. Other versions require renewed validation.
2. Open [MDK-ARM/RM_Frame_C.uvprojx](../firmware/v4/MDK-ARM/RM_Frame_C.uvprojx); the target is named `RM_Frame_C`.
3. Verify the `STM32F407IGHx` target, linker memory layout, programmer, and physical hardware connections before building.
4. The actual application entry point is `main()` in [Usermain.cpp](../firmware/v4/userCode/devices/Src/Usermain.cpp). [Src/main.c](../firmware/v4/Src/main.c) retains generated configuration such as system clocks.

The archive review checked that all source files referenced by the primary Keil project exist; it did not run a Keil build. Upstream also includes a `CubeMX/MDK-ARM` snapshot. Do not mix the relative paths or output directories of the two projects.

### CMake / GNU Arm

[CMakeLists.txt](../firmware/v4/CMakeLists.txt) configures GNU Arm cross-compilers. Install CMake, Ninja, and a GNU Arm toolchain with the `arm-none-eabi-` prefix, add them to PATH, then run from the repository root:

```sh
cmake -S firmware/v4 -B firmware/v4/build -G Ninja
cmake --build firmware/v4/build
```

The expected target is `RM_Frame_C.elf`; the build script also converts it to `RM_Frame_C.hex` and `RM_Frame_C.bin`. The linker script is [STM32F407IGHx_FLASH.ld](../firmware/v4/STM32F407IGHx_FLASH.ld). The project declares CMake 3.7 minimum, GNU99 for C, C++11 for C++, and Cortex-M4 as the CPU.

These instructions describe the upstream build entry point, not a successful build performed during this archival work. Both example soft- and hard-float switches in the current CMake file are commented out; do not assume the resulting floating-point ABI matches Keil. After changing the compiler, enabling hard float, or regenerating the CubeMX project, validate compilation, linking, startup, and peripheral behavior separately.

## Runtime organization

[ParamConfig.h](../firmware/v4/userCode/devices/Inc/ParamConfig.h) declares eight thrusters, four servos, and four pressure sensors, a 100 Hz control frequency, and a maximum serial buffer size of 128 bytes. The device-table order in [Usermain.cpp](../firmware/v4/userCode/devices/Src/Usermain.cpp) also defines serial device addresses:

| Address | Device | Current behavior |
| --- | --- | --- |
| `0x00` | IMU | In the device table; updated at approximately 400 Hz from the TIM1 tick |
| `0x01` | PressureSensor | Processed in the 100 Hz main-loop path; alternates temperature and pressure reads |
| `0x02` | Propeller_I2C | Thrusters controlled through PCA9685 PWM expansion |
| `0x03` | Servo_I2C | Servos controlled through PCA9685 PWM expansion |
| `0x04` | LED | In the device table |

Sonar, Watchdog, and Buzzer objects or implementations exist, but the current device table does not include all of them. `MX_IWDG_Init()` is commented out. Initialization in [IMU.cpp](../firmware/v4/userCode/devices/Src/IMU.cpp) still checks BMI088, while `Handle()` currently calls `h30_attitude_update()`; the local BMI088 DMA/Mahony update path is commented out. H30 data arrives through USART1. The presence of a BMI088 driver does not mean the current attitude loop depends only on the onboard BMI088.

The current H30 mapping is `roll=NormalizeAngle(h30.roll+π)`, `pitch=-h30.pitch`, and `yaw=-h30.yaw`, with angular rates taken directly from H30 `wx/wy/wz`. Validate this mapping together with the physical IMU mounting orientation.

## V40 configuration and edit locations

`Robot_Version` is set to `V40` in [Usermain.cpp](../firmware/v4/userCode/devices/Src/Usermain.cpp). The enumeration in [ParamConfig.h](../firmware/v4/userCode/devices/Inc/ParamConfig.h) contains only V30, V31, V32, V33, and V40; there is no V41 or V4-pro branch.

| Item | Current value/behavior | Source |
| --- | --- | --- |
| Inner-thruster indices | `{1,2,6,5}`; comment order: front-left, rear-left, front-right, rear-right | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| Outer-thruster indices | `{0,3,7,4}`, in the same position order | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| Propeller-direction signs | `{1,-1,1,1,-1,-1,1,-1}`, indexed by thruster number | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| Initial PWM / deadband compensation | 1590 μs; compensation is 50 on all eight channels | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| PWM frequency and output limits | 50 Hz; thruster pulse width is clamped to 1000–2000 μs when transmitted | [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| Thrust allocation | Fixed 8×6 allocation matrix and 6×8 resultant-force/moment matrix; axis comments specify forward, right, down, and roll/pitch/yaw | [Propeller.h](../firmware/v4/userCode/devices/Inc/Propeller.h) |
| Speed/force limits | 20 N on each horizontal axis; yaw rate 0.8 rad/s; internal depth-rate limit 10 | [Propeller.h](../firmware/v4/userCode/devices/Inc/Propeller.h) |
| Servo channels | Consecutive PCA9685 channels 8–11; initialization copies `ID_V40`, while interrupt-driven transmission writes consecutive addresses | [Servo.cpp](../firmware/v4/userCode/devices/Src/Servo.cpp) |
| Initial servo pulse widths | `{1550,1500,1500,1550}` μs | [Servo.cpp](../firmware/v4/userCode/devices/Src/Servo.cpp) |
| Servo start/stop | Entries 0 and 3 are set to 1650/1550 μs; transmission clamps to 1000–2000 μs | [Servo.cpp](../firmware/v4/userCode/devices/Src/Servo.cpp) |
| V40 pressure offsets | `{1018.4,1020.0,1018.5,1019.1}` | [Sensor.h](../firmware/v4/userCode/devices/Inc/Sensor.h) |
| Pressure-sensor positions | x=`{-12,12,12,-12}`, y=`{-5.4,-5.4,5.4,5.4}`, z all zero; source requires adjustment to measurements | [Sensor.cpp](../firmware/v4/userCode/devices/Src/Sensor.cpp) |
| I2C expansion | I2C2; TCA9548A channels 0–3 for pressure sensors and channel 4 for PCA9685 | [Sensor.cpp](../firmware/v4/userCode/devices/Src/Sensor.cpp), [Propeller.cpp](../firmware/v4/userCode/devices/Src/Propeller.cpp) |
| I2C address representation | TCA macro `0x70` is shifted at call sites; PCA macro `0x80` is already the shifted address used by HAL; pressure macro is `0xEC` | [Extension.h](../firmware/v4/userCode/devices/Inc/Extension.h), [Sensor.h](../firmware/v4/userCode/devices/Inc/Sensor.h) |

The actual `Robot_Version` switch selects pressure offsets. Thruster arrays, allocation matrices, and PID parameters are fixed global configuration, and servo initialization is fixed to V40. Changing the enumeration alone does not adapt the entire vehicle. The V3.x enumeration values describe historical vehicle configurations; they are not substitutes for a V4/V4-pro model mapping.

Current control includes depth PID, an outer attitude loop, an inner angular-rate loop, feedforward, and an extended state observer (ESO). PID parameters are at the beginning of `Propeller.cpp`; the feedforward/ESO `J/B/D/omega_o` parameters still carry a parameter-identification TODO. No gains, signs, or model parameters were changed during this archival work.

## Serial interfaces and protocol

The authoritative settings are in [Src/usart.c](../firmware/v4/Src/usart.c):

| Interface | Current configuration | Purpose |
| --- | --- | --- |
| USART1 | 921600, 8N1, no hardware flow control | H30 IMU reception and decoding |
| USART6 | 115200; HAL nine-bit word length, even parity, one stop bit, normally configured as 8E1 on the host | Device commands and text output |
| USART3 | 100000, 8N1 | Initialized, but not used by the primary command dispatcher in this commit |

USART6 uses receive-to-idle DMA. The callback sets a flag only when `Size>=2`. The main loop reads `RxBuffer6[0]` as a device address in the range 0–4, and the device then reads `RxBuffer6[1]` as its command. There is no common frame header, length field, CRC, sequence number, or response format. Do not indiscriminately concatenate several commands into a single receive-idle event.

### Thruster commands, device address `0x02`

“Minimum bytes” below means the data required by the handler, including address and command bytes. The current firmware does not enforce each command's required length. Sixteen-bit integers use signed little-endian encoding. Except for the closed-loop toggle and empty handler, the target/motion commands below are processed only when `flag_on` is true.

| Command | Minimum bytes | Current behavior |
| --- | --- | --- |
| `0x00` | 4 | Assign `int16_le / 10.0` to the depth target |
| `0x01` | 4 | Add `int16_le / 10.0` to the depth target |
| `0x02` | 4 | Clamp int16 to -99…99, then multiply by `MAX_DEPTH_SPEED/100` to set the depth-target rate |
| `0x10` | 2 | **Toggle** `flag_on`; this is not a pair of explicit on/off commands |
| `0x20` | 2 | Registered as a yaw closed-loop command, but its handler is empty |
| `0x21` | 4 | Convert an int16 angle from degrees to radians and assign the yaw target |
| `0x22` | 4 | Convert an int16 angle increment from degrees to radians and add it to the target |
| `0x23` | 4 | Clamp int16 to -99…99, then multiply by `MAX_ROTATE_SPEED/100` to set the yaw-target rate |
| `0x30` | 4 | byte2 and byte3 are signed int8 lateral and longitudinal inputs, clamped to -99…99 and mapped to horizontal thrust |

For example, `02 00 2C 01` carries integer 300; the handler writes a depth target of 30.0. This is an encoding example and is processed only when closed-loop control is already enabled. Do not repeatedly send `02 10` to “ensure enabled”: the second command disables it again.

The depth field is commented as cm, but `CalcRealPressure()` currently computes and filters a pressure difference without explicitly converting it using water density and gravitational acceleration. `data_depth` is the average of four channels. The `/10` above is a numerical transformation in the source, not proof of physical depth units or accuracy. Calibration must match the actual sensor and water conditions.

`CommandSetHorizontalStop()` and `CommandTest()` exist but are not registered in the command table; no usable command codes are invented for them. After closed-loop control is disabled, `Handle()` still sets each thruster to `InitPWM=1590` and transmits that value. This does not cut power or stop PWM output. Whether 1590 μs is stationary depends on the actual ESC calibration.

### Servo commands, device address `0x03`

| Command | Bytes | Current behavior |
| --- | --- | --- |
| `0x00` | 2 | Set pulse-width entries 0 and 3 to 1650 μs |
| `0x01` | 2 | Restore entries 0 and 3 to 1550 μs |

The handlers are named StartMotor/StopMotor, but their meaning depends on the installed mechanism. They are not universal servo power-on/off commands.

### Status output and historical commands

The main loop currently outputs roll, pitch, and yaw in degrees at approximately 10 Hz, with three decimal places, comma separators, and a final newline. A UART error sets `error` and stops this normal angle-output path. Error handlers, including I2C errors, can also output text/numbers on the same serial port; not every line contains three angles.

The pressure device's `Receive()` still matches bare `CA/VA` strings. Their first character is outside the valid address range 0–4, while prefixing an address prevents the string comparison at the start of the buffer from matching. These commands therefore cannot be invoked normally through the current dispatcher. The old `ON/OFF/PRO/VEL/MOT/WASD` descriptions are also not the current thruster interface. The historical [serial protocol text](../firmware/v4/串口通信协议.txt) and [README notes](../firmware/v4/说明文档Readme.txt) are retained for traceability.

## Known limitations and validation scope

The following findings come from static review of the pinned source. This import did not fix or alter them:

- **Short frames and buffer consistency:** The dispatcher requires only two bytes, while several handlers read the third and fourth bytes. Short frames can consume stale buffer contents. DMA directly reuses `RxBuffer6` without a frame snapshot or queue before the main loop consumes it.
- **Communication-loss protection:** The watchdog is disabled. There is no evidence that the default configuration stops thrusters automatically after a communication timeout. Disabling the closed loop does not electrically disconnect power.
- **Bus waits:** Thruster and servo handling contain busy waits without timeouts. The I2C error callback reinitializes the peripheral, but state-machine reset statements are commented out, creating a risk of stalling after faults.
- **Sensor initialization:** Pressure-sensor PROM/CRC initialization includes an unbounded loop. Missing sensors or persistent read failures can prevent startup from completing.
- **Calibration persistence:** Flash writes in pressure calibration are commented out. Retention of calibration across restart is not guaranteed.
- **Parameters and coordinate frames:** Thruster neutral values, deadbands, propeller signs, allocation matrix, IMU axes, and pressure-sensor positions must be checked together for the particular vehicle. Do not copy V4-pro parameters into this branch.
- **Build and hardware:** Source-import integrity and primary Keil file-reference checks are complete. Cross-compilation, flashing, sensor bench testing, and thruster tests have not been performed.

The recommended reproduction sequence is to compile for the target and establish board startup, then verify serial parsing, sensor feedback and calibration, and each actuator's direction, before testing the depth and attitude loops separately. Record the commit, toolchain, physical wiring, and parameters so the results can be attributed to one specific vehicle.
