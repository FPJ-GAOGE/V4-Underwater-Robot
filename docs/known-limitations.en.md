# Known gaps and conflicting sources

[中文](known-limitations.md) | English

This document records the state of the available material and pinned source code. This work archives and explains them; it does not functionally rewrite the original control programs.

| Area | Finding | Effect on reproduction / further evidence needed |
| --- | --- | --- |
| V4 Raspberry Pi | The usage guide names the `joystick_control` directory and records automatic startup, but supplies no server source | Obtain code, service definitions, serial settings, and dependencies from the original vehicle; the historical image example is not a substitute |
| V4-pro PC program | The Orin guide uses a matching Windows gamepad program without an identifiable source repository or attachment | Obtain the PC program matching the 41-byte UDP format; the V4 TCP client is incompatible |
| V4 firmware board | System material says C board; the current project explicitly targets STM32F407IGHx | Check the actual PCB, MCU, pins, and flashing target; filenames do not establish general compatibility |
| V4 IMU | H30 supplies feedback, but startup still checks BMI088 | Verify whether initialization completes with H30 alone |
| V4 UART | Receive code only checks length ≥2, although some commands need 4 bytes; the shared DMA buffer has no snapshot | Short frames, stale bytes, and overwrite risks exist; the serial bridge must match the actual boundaries |
| V4 control semantics | `02 10` toggles closed-loop control; `02 20` has an empty implementation; disabling closed-loop control still outputs 1590 μs | Repeated toggling is not an idempotent start command or a physical stop |
| V4 timeouts and buses | Watchdog is disabled, I²C busy-waits have no timeout, and calibration Flash writes are commented out | No automatic thruster stop on link loss or calibration persistence across reboot is claimed |
| V4 depth | Current calculations retain a pressure-difference proxy; the comments' cm unit is not rigorously established by the computation chain | Measure and calibrate actual units, offsets, and conversion |
| V4 quantities/current ratings | Parts list: 2 servos and 30 A relay; older electrical guide: 4 servos and 120 A relay | The BOM retains the conflict. Check actual components, mechanisms, and loads; no final hardware selection was made on the user's behalf |
| V4-pro PWM supply | Manufacturing text says 8.4 V; simplified diagram labels servos 5 V and main power 24 V | Do not combine these into one supply rail; verify load specifications, board jumpers, and signal levels |
| V4-pro branch assignment | pro1/pro2 differ in motors, IMU, and pressure processing; manufacturing documentation does not identify which vehicle uses which | Retain both versions and the patch without assigning a branch to a physical vehicle by assumption |
| V4-pro ESCs | Current source uses bidirectional DSHOT 3D mode; manufacturing text does not give complete ESC firmware/settings | Do not apply V4's 50 Hz PWM settings; verify ESC support and actual configuration |
| V4-pro build | Upstream MDK project omits TaskSUB.cpp; CMake depends on Windows Arm Compiler 6 | Build entry points are preserved and explained. The proprietary toolchain was not installed and no successful build is claimed |
| Jetson scripts | Native `struct` byte order; no automatic recovery after serial-open failure; no automatic stop on input loss | Static protocol agreement is not an operational guarantee |
| Video script | Missing pipeline operator in raw branch; CPU fallback retains NVMM; incorrect timeout exception type | Originals are retained; affected branches need repair and validation on Jetson with a camera |
| Startup | Original attachment contains personal paths, chmod777, and process-name cleanup | Original retained for provenance only; separate systemd templates are provided for site-specific configuration |
| Historical image demo | Server-sent length/object does not match pickle encoding; several scripts are incomplete | Retained only as historical development material, not a current-system dependency |
| Licensing | Some programs, mechanical files, and documents have no explicit redistribution license | Repository remains private; confirm ownership and license scope before public release |
| Performance figures | No complete, reviewable pressure-depth, endurance, speed, or closed-loop performance report was supplied | README contains no inferred performance figures; a sensor's 10 m range is not the vehicle's rated operating depth |

For detailed source evidence, see [V4 firmware](firmware-v4.en.md), [Pro import](../firmware/v4-pro/IMPORT.en.md), [V4 PC software](../software/v4-topside/IMPORT.en.md), and [Jetson](../software/v4-pro-jetson/README.en.md).
