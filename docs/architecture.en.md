# System architecture

[中文](architecture.md) | English

These diagrams are based on the reviewed system documentation and pinned source code. Arrows indicate data/control direction; they do not replace electrical schematics.

## V4: Raspberry Pi + C board

```mermaid
flowchart LR
  J[PC gamepad / Pygame] -->|TCP 5000, device address + command| P[Raspberry Pi joystick_control\nCurrent server source not provided]
  P -->|UART6 115200 8E1| C[IWIN-FINS V4-ly / V40]
  H[H30 IMU] -->|USART1 921600 8N1| C
  C -->|I2C2| M[TCA9548A]
  M -->|Channels 0 to 3| S[4 pressure sensors]
  M -->|Channel 4| W[PCA9685]
  W -->|PWM 0 to 7| T[8 thrusters]
  W -->|PWM 8 to 11| V[4 firmware servo outputs]
  P -. Video sender not provided .-> PC[PC video receiver]
```

The PC byte protocol corresponds to the firmware command table. However, TCP is a byte stream, while the firmware dispatches commands through UART idle interrupts; the missing Raspberry Pi server's treatment of boundaries and timing cannot yet be verified. Four servo channels are a source-code capability, not evidence that a particular vehicle has four installed servos. The procurement list and older wiring guide disagree on quantity.

## V4-pro: Jetson + A board

```mermaid
flowchart LR
  PC[Matching PC gamepad program\nSource not provided] -->|UDP 12345, 41 bytes| J[Jetson V5StreamerNX.py]
  J -->|UART3 115200 8N1\n28-byte control frame with CRC| A[FineSUB pro1 / pro2]
  A -->|UART data forwarded unchanged| J
  J -->|UDP 54321| PC
  IMU[H30 IMU] -->|UART7 921600 8N1| A
  A --> DSHOT[8 DSHOT channels, bidirectional 3D mode]
  DSHOT --> E[2 four-in-one ESCs]
  E --> T[8 brushless thrusters]
  A --- IIC[I2C expansion and 4 pressure sensors]
  CAM[USB camera] --> GST[Jetson GStreamerTest.py]
  GST -->|RTP/H.264 UDP 5600| V[PC video receiver]
```

Static inspection shows that the Jetson script and FineSUB use matching 28-byte control-frame formats; this is not proof of a working hardware link. FineSUB uses DSHOT for thrusters and separate PWM outputs for servos and lights. A board port being labeled PWM does not mean that the ESC protocol is ordinary pulse-width control. See the [Jetson guide](../software/v4-pro-jetson/README.en.md) for the complete protocol, native-byte-order dependency, and deployment parameters.

## Where to change configuration

| Change | V4 | V4-pro |
| --- | --- | --- |
| Thruster order, directions, initial values, controllers | `firmware/v4/userCode/devices/Src/Propeller.cpp` and its corresponding `.h` | `firmware/v4-pro/Interface/Examples/TaskSUB.cpp`; also see the pro2 patch |
| Pressure offsets, sensor locations, depth processing | `Sensor.cpp` / `Sensor.h` | `TaskSUB.cpp` and the MS5837 drivers |
| IMU mounting orientation | H30 mapping in `IMU.cpp` | `rotationOffset` in pro1/pro2 |
| PC address and gamepad mapping | `software/v4-topside/pc_client*.py` | Fixed UDP / serial parameters in the Jetson script; a matching PC program is required separately |
| Onboard startup | Original systemd service was not provided | Example templates in `software/v4-pro-jetson/deploy/` |

When reusing components across models, establish command semantics, units, and coordinate frames before validating hardware channels. The presence of V4, V5, or SUB in a filename is not enough to establish compatibility.
