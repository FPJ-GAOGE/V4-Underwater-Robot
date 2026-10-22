# Operation and deployment

[中文](operation.md) | English

First establish [which model](variants.en.md) matches the vehicle's boards, software, and wiring. The commands below are reproduction entry points; no vehicle was connected to run these programs during this repository preparation.

## V4

1. Use the [V4 hardware guide](hardware-v4.en.md) to check the C board, Raspberry Pi, I²C/PWM expansion boards, and power supply. Build the user-specified `V4-ly` firmware following the [firmware guide](firmware-v4.en.md).
2. Put the Raspberry Pi and PC on the same subnet. The historical configuration uses `192.168.138.2` for the Raspberry Pi and `192.168.138.10` for the PC; these are example addresses and should be adjusted for the actual network interface.
3. Log in with your own configured SSH account. The shared account and password in the original material are not distributed with this repository.
4. The original onboard service directory is named `joystick_control`. The documentation says it starts through systemd, but supplies neither the service name, scripts, nor forwarding implementation. Confirm that the service exists on the original vehicle and record its startup procedure. Without this service, the PC client alone does not form a complete control link.
5. Configure a Python environment on the PC and follow the installation commands in the [topside import guide](../software/v4-topside/IMPORT.en.md). `ROV-V4` in the original guide is merely the name of an existing environment, not a reproducible environment file supplied by this repository.
6. Connect the gamepad to the PC, then start `pc_client.py` from `software/v4-topside`. Inspect gamepad axis IDs, directions, and target values before allowing thruster output. The plotting client, `pc_client_plot.py`, also sends control commands.

The firmware specifies UART6 at 115200 8E1. The Raspberry Pi's actual serial device and forwarding behavior still require confirmation from the original onboard program. `02 10` toggles state; sending it again disables the state again. Client exit does not send an explicit stop frame, so closing the window must not be treated as stopping the thrusters.

### V4 troubleshooting

| Symptom | Check first |
| --- | --- |
| Cannot connect over TCP | Subnet, port 5000, whether the original onboard service is running, and whether another client occupies it |
| Connected but no attitude text | Serial device, 115200/8E1, UART6 wiring, firmware startup, and sensor initialization |
| Vehicle rotates at startup or moves in an unexpected direction | Gamepad axis IDs/signs, IMU mounting orientation, actual thruster channels and propeller directions, and closed-loop toggle state |
| No video | Whether the camera service exists, destination IP, and sender/receiver ports; the upstream SDP uses 5004, while the GStreamer example uses 5600 |

The original usage record reports recovering missing serial output by reflashing, but does not establish the cause. This is not a generally reliable fix. Preserve fault information and inspect the link first; repeated reflashing is not documented here as a required startup step.

## V4-pro

1. Wire the A board, Jetson, IMU, ESCs, and sensors using the [V4-pro hardware guide](hardware-v4-pro.en.md).
2. Select [FineSUB pro1 / pro2](../firmware/v4-pro/variants/README.en.md) according to the actual vehicle. Check ports, directions, and parameters before building. Do not assume an instance is pro1 or pro2 without physical evidence.
3. Start integration with a direct Ethernet cable between Jetson and PC. The documentation examples use Jetson `192.168.0.2` and PC `192.168.0.1`. The forwarding script, video script, and PC must use consistent addresses.
4. Connect the camera over USB and Jetson `/dev/ttyTHS1` to A-board UART3. Connect ground to ground, TX to RX, and RX to TX; remove the 5 V power wire from this serial cable. Device nodes depend on the actual carrier board, not solely on the Orin model.
5. Install dependencies and manually test control forwarding and video according to the [Jetson README](../software/v4-pro-jetson/README.en.md). The original startup script hardcodes personal directories and is retained as a historical record. Prefer the separate service templates under `deploy/`, after adjusting paths, user, and permissions.
6. The matching PC gamepad data must follow the 41-byte UDP format; the current V4 TCP client is incompatible. Observe receive state, channel targets, and sensor values with a debugger before conducting constrained actuator checks.

For video reception, use the H.264 / UDP5600 example in the Jetson README. The original GStreamer script has documented problems in its raw branch and CPU fallback. Without a camera and Jetson, only command construction can be checked; video operation cannot be claimed.

## Common assembly and leak-check sequence

Inspect machined parts and seals first, then complete electrical connections outside the enclosure. Verify polarity, supply voltages, ESC neutral behavior, and sensor feedback before closing the enclosure. Following the original manufacturing record, remove the battery for the initial still-water leak check, and reinstall it only after confirming there are no leaks. This is not a pressure test to a rated operating depth. Record hardware, firmware commit, exact configuration, calibration, and faults during testing; results from different vehicles are not interchangeable.
