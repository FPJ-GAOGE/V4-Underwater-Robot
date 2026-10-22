# V4-pro Hardware and Manufacturing

[中文](hardware-v4-pro.md) | English

The V4-pro sources describe an **RM-A controller, Jetson Orin onboard computer, two four-in-one ESCs, eight thrusters without integrated ESCs, four pressure sensors, two servos, and two lights**. This page consolidates the V4-PRO manufacturing document for the RM-A controller, the simplified wiring diagram, and the Orin setup notes. Its power and control paths differ from the [RM-C/PCA9685 arrangement used by V4](hardware-v4.en.md).

## Models and machined parts

Engineering files are available in [V4-pro CAD](../hardware/v4-pro/cad). The complete `V4pro.zip` and native SolidWorks files are stored in the [mechanical source release](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/releases/tag/mechanical-source-2026-10-22). The [asset manifest](../hardware/assets-manifest.json) records provenance and checksums. A filename containing `V4外形` (“V4 exterior”) still belongs to the source `V4pro.zip`; the name does not make it a part of the older V4 revision.

| Aluminum-alloy machined part | Quantity in manufacturing document | CAD location |
| --- | --- | --- |
| Side support | 2 | [STEP](../hardware/v4-pro/cad/主体/侧支撑.STEP), [DWG](../hardware/v4-pro/cad/主体/侧支撑.DWG) |
| Bottom support, filename `底支撑2` | 2 | [STEP](../hardware/v4-pro/cad/主体/底支撑2.STEP), [DWG](../hardware/v4-pro/cad/主体/底支撑2.DWG) |
| Upper thruster support | 2 | [STEP](../hardware/v4-pro/cad/主体/上推进器支撑.STEP), [DWG](../hardware/v4-pro/cad/主体/上推进器支撑.DWG) |
| Front gimbal base | 1 | [STEP](../hardware/v4-pro/cad/相机云台/前云台底架.STEP), [DWG](../hardware/v4-pro/cad/相机云台/前云台底架.DWG) |
| Rear end cap | 1 | [Rear end cap 165 STEP](../hardware/v4-pro/cad/密封舱/后舱盖165.STEP), [DWG](../hardware/v4-pro/cad/密封舱/后舱盖165.DWG) |

Select the acrylic tube, seals, flanges, and mounting hardware against the complete assembly. The quantities above come from the manufacturing list; they are not a full exploded procurement list of every CAD part. Missing material grades, tolerances, finishes, and sealing requirements must be resolved using the applicable engineering drawings and supplier requirements.

The source calls for one printed upper board tray, one lower tray, four pressure-sensor brackets, and two light brackets. See the [internal trays](../hardware/v4-pro/cad/密封舱/舱内), [pressure-sensor brackets](../hardware/v4-pro/cad/水深传感器), and [light brackets](../hardware/v4-pro/cad/外设). Heat-set inserts use **M3 or M4** according to the model, unlike the M2.5/M3 description for V4 trays.

## Thrusters and custom external harnesses

| Device | Quantity and specification | Cable/connector requirements in the source |
| --- | --- | --- |
| Thrusters | Eight without ESCs; four of each opposite propeller handedness | One of each handedness per pair. The four pairs use **65, 50, 38, and 32 cm** waterproof cables; 1W three-pin plugs, with mating sockets specified at 15A and 20 cm internal leads |
| Pressure sensors | Four, 316L stainless steel; selected sensor range 10m | **70 cm** waterproof cable, 00W four-pin connector; pin 1 red, 2 black, 3 green SCL, 4 white SDA; socket leads 20 cm |
| Lights | Two PWM-controlled lights, source beam angle 100° | 1W three-pin connector, 60 cm waterproof cable; pin 1 red, 2 black, 3 yellow PWM; socket specified at 3A with 20 cm leads |
| Servos | Two | 1W three-pin connector, 75 cm waterproof cable; pin 1 red, 2 black, 3 yellow PWM; socket specified at 2A with 20 cm leads |

These socket current labels are source manufacturing requirements, not an overall current budget or measured continuous-current capability. Specify whether a pin drawing shows a plug or socket and its mating or solder side, then verify continuity pin by pin after assembly.

**The three Pro thruster wires are the three motor phases from the ESC to the brushless motor.** The source does not mandate a phase-wire order, but exchanging phases affects rotation direction, so verify every motor's rotation and propeller handedness. These are different from V4's positive-supply, negative-supply, and PWM wires; the two harness types are not interchangeable based on red/black/yellow colors.

## Mechanical and internal electrical assembly

1. Install thrusters on the side and upper supports. The source specifies flat-head screws on the side supports. Keep labels linking propeller handedness, cable-length pair, and mounting position.
2. Install sockets in the threaded rear end cap. The original arrangement does not use separate nuts. Follow the connector, end-cap thread, and seal requirements for fastening; “tighten” does not remove the need to consider torque and sealing surfaces.
3. Install the pressure-sensor brackets and lights on both sides. Check that harnesses do not interfere with thrusters, the gimbal, or enclosure removal.
4. Install M3/M4 inserts at the specified holes in the upper and lower trays, then trial-fit the RM-A, Jetson, distribution board, two ESCs, I2C expansion board, IMU, and terminal blocks.
5. Prepare ESC power and capacitor connections, distribution branches, the custom battery connection, rotary switch, and power-line communication harnesses. The source includes shortening the battery connector; assemble a completed, verified harness and do not alter energized wiring.
6. The source repurposes the A-board OLED power connector for the I2C expansion board and explicitly specifies 3.3V for the pressure-sensor path. Identify 3.3V and GND from the actual A-board pinout rather than relying on the port name alone.
7. Trim internal socket leads while retaining service allowance. Prepare the camera penetration, organize and secure the harnesses, and trial-fit the complete internal assembly.

## Original diagram and conflicting voltages

![Original V4-pro simplified wiring diagram, with voltage and terminal annotations requiring verification](images/v4-pro-wiring-board.jpg)

The original diagram labels the custom main supply **24V** and the servo supply **5V**, whereas the manufacturing text instructs setting the A-board “adjustable PWM voltage output” to **8.4V**. The light power wires are also marked 24V in the drawing. These statements cannot be combined into an instruction that every PWM device uses 8.4V or that all of them use 5V.

Before assembly, record the power specifications of the actual servos, lights, A board, Jetson carrier, and ESCs. Measure main power, actuator power, and logic power separately. PWM control signals, the board's adjustable load supply, and the 24V power rail are different electrical functions. This repository does not resolve the 5V/8.4V discrepancy to one universal setting without the hardware specification. The original drawing remains available with this discrepancy explicitly documented for comparison against the vehicle.

The drawing also shows a 20P signal terminal, a 12P servo terminal, and two 12P motor terminals, while the procurement text lists only “12P × 2.” Recalculate terminal quantity, pole count, and current requirements from the final harness schedule; the drawing and text are not already consistent.

## Controller, Jetson, and communication

| Connection | Source requirement | Verification point |
| --- | --- | --- |
| Jetson → RM-A | `/dev/ttyTHS1` on the specified Damiao carrier connects to RM-A **UART3** | This device name and physical location apply to the documented carrier; check other versions against their manuals |
| Serial cable | Remove the **5V supply wire** from the GH1.25 four-pin cable; retain GND↔GND, TX↔RX, RX↔TX | Do not power one board from the other through this serial cable |
| Camera → Jetson | USB, using a carrier-board USB-C port in the source | Verify connector type and power capability for the actual carrier |
| PC → Jetson | Ethernet, with a paired power-line communication link in the assembled vehicle | Commission direct Ethernet first, then introduce the power-line modules to isolate link problems |
| IMU → RM-A | IMU settings **921600, 400Hz** | The source requests the corresponding firmware from the supplier; verify the output after flashing rather than relying only on an indicator LED |
| I2C expansion → pressure sensors | Expansion board connects to RM-A at **3.3V**; four pins are positive, negative, SCL, and SDA | Verify ground reference and channel assignments |
| Debugger → RM-A | Wireless SWD programmer | The list contains two units; this does not mean two debuggers should connect to one board simultaneously |

The Orin notes cover Orin NX and state that Orin Nano is also supported; the carrier-specific instructions refer to a Damiao carrier. “NANO” must not be generalized to an arbitrary older Jetson Nano model. Default credentials, fixed workstation paths, and fixed network addresses from the source are not repository configuration.

## Individual-device commissioning and full-system verification

The archived FineSUB pro1/pro2 source drives all eight thrusters through **DSHOT**, with bidirectional 3D mode and telemetry enabled. Servos and lights use PWM pulse-width control. The original manufacturing prose names board connector letters without specifying the complete ESC firmware and protocol configuration. Match ESC settings to the pinned firmware; a connector being called a PWM port does not mean the thrusters use 50 Hz analog PWM. See [firmware entry points and protocol](../firmware/v4-pro/IMPORT.en.md).

1. **Power and controllers:** Use a controlled constant-voltage supply with the assembly outside the enclosure to check each rail, RM-A and Jetson startup, serial communication, and programming interfaces. Record firmware and onboard-program versions.
2. **IMU and pressure:** Configure IMU baud rate and output frequency, and inspect attitude and angular-rate data. Connect the four pressure sensors individually and record offsets and physical positions. Sensor measurement range is not an enclosure pressure rating.
3. **ESCs and thrusters:** Complete the four-in-one ESC settings and all eight signal connections. The manufacturing document labels these interfaces `Z~S`; identify them individually using actual A-board markings and firmware timer channels. “Upper-left is number 1, upper-right is number 8” is not a complete channel map.
4. **Lights and servos:** The source assigns lights to A/B and servos to C/D. Resolve the 5V/8.4V and power-rail discrepancies above first. Set the servos to their zero position before mounting the camera to align mechanical and control zero.
5. **Onboard communication:** Confirm gamepad commands reach the RM-A before checking video and the power-line link. Run programs manually and record serial/network settings before enabling startup services, then verify recovery after reboot. Do not start duplicate programs on the same serial port.
6. **Vehicle attitude:** The original text describes upper thrusters as the vertical group and lower thrusters as the horizontal group, and refers to a `V5_sub` thrust matrix. This historical software name does not change the mechanical model from V4-pro or justify copying a V4-ly matrix. Verify physical numbering, rotation, and firmware output mapping before deciding whether the matrix needs changes.

## Enclosure closure and leak checks

The original procedure is to assemble the acrylic tube, insert the internal electronics with the **battery removed**, place the assembly in water to observe leakage, and reinstall the battery after the check. Keep the electronics unpowered. Inspect the end caps, penetrators, camera cable penetration, seals, and potted connectors; inspect and dry the assembly after removal, and correct any leaks before proceeding. Then perform a shore-side control test followed by a controlled in-water test.

This repository archives manufacturing information and models. No new hardware, leak, or pressure tests have been completed. The sources do not provide a uniform rated operating depth, pressure-hold duration, or acceptance pressure. Main-supply voltage, a sensor's 10m option, and material names are not evidence of verified system performance.
