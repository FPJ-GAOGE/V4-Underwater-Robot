# Differences between V4 and V4-pro

[中文](variants.md) | English

Here, **V4** means the Raspberry Pi / RM-C system documented in “Competition Details and V4 Design” and “V4 User Guide,” with the control firmware pinned to the specified IWIN-FINS `V4-ly` branch. **V4-pro** means the system described in “V4-PRO Manufacturing Documentation (RM-A Controller)” and its Orin setup reference. FineSUB `v4pro1` / `V4pro2` are two configurations within the Pro family; they must not be substituted for the V4 / V4-pro comparison.

## Mechanical structure and electrical system

| Component | V4 | V4-pro | Differences to account for when reusing parts |
| --- | --- | --- | --- |
| Frame and internal board mounts | V4 models dated 2025-05-23, arranged for the C board / Raspberry Pi | Separate V4pro models, arranged for the A board / Jetson / ESCs | Parts with the same name are not necessarily interchangeable; use the dimensions in each model set |
| Later mechanical parts | Monocular camera mount dated 2025-07-10; a separate gripper design attachment | Camera gimbal with two servos and light mounts | The gripper attachment is design material, not proof of integrated gripper control |
| Onboard computer | Raspberry Pi 4B | The manufacturing list says NANO; the linked setup guide explicitly covers Orin Nano / NX | Serial device names, carrier-board pins, and software environments differ |
| Controller | System documentation specifies RM-C; the supplied project targets STM32F407IGHx | RM-A / STM32F427 | Check the actual board, MCU, and project; do not interchange firmware images |
| Thruster count | 8 | 8: 4 standard and 4 reverse propellers | Equal counts do not imply equal channels or directions |
| Thruster drive | Documented three-wire connection: power positive, power negative, and PWM; PCA9685 outputs 0–7 | Thrusters without integrated ESCs plus 2 four-in-one ESCs; three motor phase wires | **The three-pin connectors have different meanings.** Do not wire Pro phase leads according to V4's red/black/yellow power definition |
| Actuator signals | I²C expansion channel 4 → PCA9685, 50 Hz PWM | Current FineSUB firmware uses bidirectional DSHOT 3D mode for thrusters and PWM for servos/lights. The manufacturing guide labels thruster connectors Z–S | Establish the relationship between board letters, DSHOT IDs, source indices, and top-view numbering; ESCs must support and be configured for the selected protocol |
| Pressure sensors | 4; the parts list specifies stainless steel, 10 m range, and 65 cm leads | 4; 316L stainless steel, 10 m range, and 70 cm leads | Sensor range is not the vehicle's pressure-depth rating; calibrate each vehicle independently |
| Servos | Procurement list: 2; older wiring documentation and firmware: 4 channels | Manufacturing list: 2, connected to A-board C / D ports | V4 sources conflict on quantity; confirm the actual mechanism |
| Lights | The supplied V4 list does not establish a lighting configuration equivalent to Pro | 2; manufacturing guide specifies A / B ports and a 100° beam angle | Do not apply the Pro port table directly to V4 |
| Main power | 4S battery; older documentation records 10000 mAh / 25C | Wiring diagram labels a custom 24 V supply | Exact Pro battery capacity and cell-series count are not provided |
| Relay | Parts list: 30 A; older electrical documentation: 120 A | Custom battery, rotary switch, and power-distribution board arrangement | The two V4 ratings conflict; neither should be treated as a verified rating by assumption |
| Servo/PWM supply | Older diagram shows a 5 V path | Simplified diagram labels servos 5 V; manufacturing text calls for the adjustable PWM supply to be set to 8.4 V | Pro sources conflict. Verify the loads, signal levels, and board jumpers first; do not connect 8.4 V to a logic input |
| Surface communication | Ethernet tether | Ethernet; manufacturing list also includes a pair of power-line communication modules | As suggested by the Orin guide, start initial integration with a direct Ethernet cable to isolate problems |

For machining, connector pin assignments, and assembly steps, see [V4 hardware](hardware-v4.en.md), [V4-pro hardware](hardware-v4-pro.en.md), and the [BOM](bom.en.md).

## Control and communication

| Component | V4 / IWIN-FINS V4-ly | V4-pro / FineSUB |
| --- | --- | --- |
| Firmware organization | Bare-metal main loop and interrupts, with a device-pointer table | FineSUB component, device, and task framework |
| PC → onboard computer | TCP 5000, carrying device-command bytes | UDP 12345, carrying 41-byte raw gamepad data |
| Command UART | UART6, 115200, 8E1 | UART3, 115200, 8N1 |
| Serial frame | Device address, command, optional payload; no common header/CRC/length field | `AA` + 25-byte payload + 2-byte Modbus CRC16, 28 bytes total |
| Data meaning | Separate commands for each device; 16-bit payloads are little-endian; `02 10` toggles closed-loop control | 5 floats, 3 uint8 values, and 2 int8 values describing the overall gamepad state |
| IMU serial port | USART1, 921600, 8N1 | UART7, 921600, 8N1 |
| Attitude-axis mapping | Adds π to H30 roll and negates pitch/yaw; initialization still checks BMI088 | Pro1 and Pro2 use different `rotationOffset` values |
| Pressure combination | Current source averages four depth proxies; physical units require calibration | Pro1 averages measurement indices 1/3; Pro2 uses index 3 only, using source-array indices |
| Actuator mapping | Fixed V40 arrays, PCA9685 channels, and an initial value of 1590 μs | FineSUB DSHOT IDs and bidirectional 3D mode; Pro1/Pro2 also differ in port assignments and signs |
| Matching PC client | `rov-v4-wired-joystick` is included | The supplied materials do not provide identifiable matching Windows source code |
| Onboard relay software | Source for the vehicle's `joystick_control` service was not provided | V5StreamerNX.py and GStreamerTest.py are included |

The V4 firmware's `Robot_Version` values V30/V31/V32/V33/V40 are historical configurations; there is no configuration named V4-pro in that enum. Merely selecting V40 or copying Pro's allocation matrix does not port the system across platforms.

## Further differences between Pro1 and Pro2

This repository retains `v4pro1` as a source baseline for comparison and provides an exact `V4pro2` patch. **There is no evidence that automatically associates either branch with a particular vehicle currently owned by the user.** The branches differ in IMU orientation, pressure-measurement selection, motor ports and thrust signs, and yaw-input scaling, deadband, and limits.

For application instructions, exact source differences, and pinned commits, see the [V4-pro import record](../firmware/v4-pro/IMPORT.en.md) and [Pro configuration notes](../firmware/v4-pro/variants/README.en.md). These are vehicle-specific calibration configurations; individual parameters should not be copied in isolation.
