# V4 Hardware and Assembly

[中文](hardware-v4.md) | English

The V4 documentation describes an **RM-C controller, Raspberry Pi 4B, PCA9685 PWM expansion, eight thrusters, and four pressure sensors**. This page consolidates the V4 design notes, AUV electrical design and component list, and the mechanical and parts tables. For actual firmware behavior, see [V4 control firmware](firmware-v4.en.md). The RM-A controller, internal four-in-one ESCs, and Jetson wiring used by V4-pro are covered in its [separate hardware guide](hardware-v4-pro.en.md).

![V4 design overview](images/v4-design.png)

## Select a mechanical revision

| Source revision | Meaning | Repository location |
| --- | --- | --- |
| 2025-05-23, refined design | Latest complete V4 model in the mechanical table; the complete mechanical baseline archived here | [V4 CAD](../hardware/v4/cad) |
| 2025-07-10, monocular camera mount | A later, separate mount package, not a complete vehicle revision | [Monocular camera mount](../hardware/v4/camera-mount) |
| Manipulator supplement | A separately preserved mechanism design; the source still lists unfinished work, so full integration with the vehicle above is not claimed | [Manipulator models](../hardware/v4/manipulator) |

The mechanical table also records a base model dated 2024-12-14, camera-gimbal and housing changes dated 2025-02-10, a new frame dated 2025-02-21, and rule-related frame changes dated 2025-02-25. Select one complete revision and explicitly identify any later parts added to it. Do not mix parts from different dates merely because their archives share the name `V4.zip`.

STEP/DWG files are available in the CAD directory. Original SolidWorks projects and attachment archives are stored in the private repository's [mechanical source release](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/releases/tag/mechanical-source-2026-10-22). The [asset manifest](../hardware/assets-manifest.json) records archive and file checksums. The original design included a stereo camera, while the later mount is for a monocular camera; confirm camera dimensions, mounting holes, and power requirements against the selected revision.

## Procurement and machining preparation

Select components using the [BOM](bom.en.md), then check their mounting holes, cable exits, connector diameters, and clearance in the complete assembly. The source machining workflow is to submit STEP files for aluminum-alloy parts and select a matte black anodized finish. Submit the corresponding DWG for parts with threaded holes. Material grade, dimensional tolerances, and thread requirements must follow the applicable engineering drawing; an exterior rendering cannot supply missing specifications.

Printed parts include internal board trays, sensor brackets, and camera connections. Check the selected CAD hole patterns against the actual board revision. The sources do not provide a uniform printing material, layer height, infill, or pressure qualification. A printed part should not be assumed to function as a pressure vessel. The two SLDPRT files dated 2025-07-10 remain a separate camera-mount update.

Determine custom cable lengths after a trial assembly of the frame and thrusters. The V4 parts table allows thrusters and servos to be sent to the connector supplier for termination. Specify device model, mating plug/socket pair, pin drawing, cable-length definition, and termination allowance; “three-core” alone is insufficient. Pressure sensors use external extension tubes. The parts table specifies **00W four-pin plugs and sockets with 65 cm four-core cables**, distinct from the 70 cm Pro requirement.

## Frame, cables, and internal assembly

1. Assemble the aluminum frame and install all eight thrusters according to the selected model. Check camera-gimbal travel and cable routing before finalizing each cable length.
2. Terminate underwater connectors according to the actual functions of the thruster and servo wires. The original V4 thruster scheme uses **positive supply, negative supply, and control signal**, recorded as red, black, and yellow. Verify the actual connector mating-face pin numbers; do not apply Pro's three-phase motor wiring.
3. Extend each pressure sensor through a sensor–external tube–watertight connector arrangement. Secure solder joints and pot them with epoxy according to the connector and potting-material instructions. The source does not specify mixing ratio, curing time, or pressure rating; use the supplied material documentation.
4. Install the mating sockets in the rear end cap. Check seals, cable bend radii, and internal service allowance, then arrange the terminal blocks.
5. Fit heat-set threaded inserts in the specified holes in the printed trays. The source specifies M2.5 for the Raspberry Pi and PWM expansion board, and M3 at other relevant positions. Not every hole takes an insert.
6. Install nylon standoffs according to the tray layout. The source says the C board and terminal blocks do not require the same standoffs; Raspberry Pi/PWM board positions use M2.5, and other applicable positions use M3. Do not convert every hole to one size.
7. After fixing the boards in their layers, remove the terminal strips temporarily for wiring, leave enough cable length, and reinstall them. Connect the switch control wires to the relay control terminals and label power and low-level signal paths separately.
8. Complete electrical checks and individual-device commissioning with the assembly exposed before inserting it into the sealed enclosure. This makes loose terminals and inaccessible connectors easier to find.

![V4 frame and cable assembly reference](images/v4-assembly.jpg)

## Electrical connections

The [original electrical PDF](../hardware/v4/electrical-connection.original.pdf) and diagram below are retained for traceability. They and the current code describe different stages of the design. Differences are listed explicitly; the historical drawing is not presented as a verified final wiring schedule.

![Original V4 electrical diagram](images/v4-electrical-board.jpg)

| Connection | V4 source description | Check during reproduction |
| --- | --- | --- |
| C board → I2C expansion | The source maps C-board I2C pins 2, 4, 6, and 8 to GND, VCC, SCL, and SDA | Verify numbering against the actual C-board manual; do not confuse VCC with actuator power |
| I2C expansion → pressure sensors | Outputs 0–3; GND/VCC/SCL/SDA correspond to black/red/green/white | Also verify sensor positions, connector pin numbers, and firmware indices |
| I2C expansion → PCA9685 | Output channel 4, wired GND/VCC/SCL/SDA | The current firmware also selects TCA channel 4 |
| PCA9685 logic/load power | The source connects 5V from a C-board PWM port to PCA9685 V+, with logic VCC supplied on the I2C side | V+ and logic VCC serve different functions; the power topology must match the purchased board |
| Thruster control | PWM outputs 0–7 connect to eight thruster signal wires | Motor power uses the power circuit, not the PWM signal pin |
| Servo control | The old electrical document uses channels 8–11 with GND/V+/PWM | The parts table lists only two servos; the old document and firmware retain four channels, as noted below |
| 4S battery → switching path | Battery, relay/switch, power branches, and buck converter; the C board and thrusters connect to the power circuit | Size components using actual current, wiring, and protection requirements, not the historical relay name alone |
| Buck converter → Raspberry Pi/servos | The source supplies the Raspberry Pi and two servos through branches; the Raspberry Pi uses 5V | Check servo operating voltage independently; do not assume every load uses the same voltage |
| SWD | Match SWCLK, SWDIO, GND, and 3.3V to the debugger | Verify target-board power and debugger connection arrangements |
| UART6 | TX↔RX, RX↔TX, GND↔GND | Current `V4-ly` uses 115200, 8E1; other historical serial rates do not override the current firmware |
| H30 IMU → USART1, added from source review | Current firmware uses H30 attitude feedback; USART1 is 921600, 8N1 | H30 is absent from the original procurement table. Confirm TX/RX/GND and power for the actual board; do not copy Pro's UART7 connection. Startup still checks BMI088 |
| Raspberry Pi communication | GPIO UART and USB-to-TTL are alternative source arrangements | The old description mixes physical pin numbers and GPIO labels. Use the actual Raspberry Pi pinout and enabled UART rather than reproducing those pin numbers verbatim |

Use completed connectors and the branch design when making battery harnesses. The old document incorrectly calls XT60 a “balance connector”; the XT60 main power connection and the battery balancing connector are different. This guide does not reproduce the old instruction to cut battery leads.

## Thruster and pressure-sensor numbering

The source explicitly defines the following diagrams as **top views looking down at the vehicle**. Establish the vehicle's forward direction, label physical numbers, and separately record their mapping to PCA9685 channels and zero-based source indices. Physical labels 1–8 are not automatically identical to array indices 0–7.

![V4 thruster positions, top view](images/v4-thruster-layout.png)

![V4 pressure-sensor positions, top view](images/v4-pressure-layout.png)

The current firmware uses inner-thruster indices `{1,2,6,5}` and outer-thruster indices `{0,3,7,4}`. Check these together with propeller-direction signs and the thrust-allocation matrix. After confirming physical placement, verify the actual effect of each positive input; do not mask a numbering error by arbitrarily changing a sign.

## Source discrepancies that require explicit resolution

| Item | Conflicting evidence | Repository treatment |
| --- | --- | --- |
| Servo count | Parts table: 2; old electrical list/channel description: 4; firmware: `SERVO_NUM=4` | Preserve both documented counts; select the installed count for the actual camera/mechanism revision and check unused channels |
| Relay | Parts table: “30A relay”; old electrical document: “120A relay” | Do not select the larger or smaller figure automatically; check the actual module and power-branch design |
| Terminal blocks | Parts table specifies 20P/16P/6P models; old prose describes groups of six- and seven-pin sockets | These represent different layout stages. Match CAD, mounting holes, and current harnesses rather than adding the quantities together |
| Camera | Complete design includes a stereo arrangement; a separate monocular mount is dated 2025-07-10 | Record the complete-model date separately from the mount-update date |
| Neutral PWM | Old protocol: 1500 μs; current firmware: 1590 μs | Use the selected firmware and actual ESC calibration; the old value is not a verified stop setting |
| Controller name | System documents say RM-C; source project targets STM32F407IGHx | Preserve both facts; the name alone does not establish compatibility with every RM-C revision |

## Power-up, enclosure closure, and leak checks

Keep the enclosure accessible initially. Measure each supply rail and polarity, check harness continuity and shorts to ground with power disconnected, and commission branches using a controlled supply. Verify the controller, onboard computer, serial communication, IMU, and all four pressure channels before checking individual thruster/mechanism directions, actual outputs after control is disabled, and the method of cutting power. Disabling a software closed loop does not cut thruster power.

After confirming that connector potting has fully cured and that end-cap sealing surfaces and seals are in suitable condition, begin with an observational leak check **with the battery removed and the system unpowered**. After removal from the water, inspect the enclosure and each penetration, dry the assembly, and resolve any leaks before powered tests. This sequence is an editorial recommendation, not a record of a completed leak test. The sources do not specify a vehicle depth rating, pressure-hold duration, or acceptance criteria. A sensor's “10m” measurement range is not a vehicle depth rating.

Record the assembly revision, actual pin assignments, measured supply voltages, cable lengths, thruster/pressure-channel mapping, firmware commit, and test results so that maintenance records correspond to this specific V4 vehicle.
