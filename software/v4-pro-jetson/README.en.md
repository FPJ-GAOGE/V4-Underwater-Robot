# V4-pro Jetson Orin NX reference programs

[中文](README.md) | English

This directory organizes three attachments from the Feishu guide “ORIN NX Setup (also applicable to ORIN NANO).” They support the **V4-pro Orin NX → ROBOMASTER A-board UART3** link. `V5` in the script names is historical naming and is not used to relabel the vehicle model.

**`upstream/` preserves historical reference files; it is not an accepted, recommended deployment implementation.** These imported files were not run on a real Jetson, camera, gamepad, or RM-A. Check their protocol and parameters against the target firmware before preparing deployment copies. The V4 Raspberry Pi / RM-C / TCP 5000 client is incompatible with this UDP gamepad protocol and cannot be substituted directly.

## Source and license

Source document: [ORIN NX Setup (also applicable to ORIN NANO)](https://iwin-fins.feishu.cn/docx/S8I9dBMNmo2dIPxhWzkcyeQsnoP). Import date: 2026-10-22. The attachments have neither Git commit IDs nor explicit open-source licenses. This directory does not assign MIT or another license on behalf of the authors; authorization for public distribution must be confirmed as part of the project's overall rights review.

| File | Purpose |
| --- | --- |
| `upstream/V5StreamerNX.py` | Receives PC gamepad packets over UDP and converts them into serial control frames; forwards received serial chunks unchanged to PC UDP |
| `upstream/GStreamerTest.py` | Captures MJPEG/RAW from a USB camera, encodes H.264 with GStreamer, and streams over RTP/UDP |
| `upstream/StreamerStart.sh` | Original combined startup script, retained only as a historical reference |
| `requirements.txt` | Python serial-bridge dependency: `pyserial` |
| `deploy/*.service.example` | Two new, separate systemd templates; they describe process management, not proof that the source programs suit the current hardware |

The original startup script hardcodes its machine's Conda directory, Python 3.8, working directory, and environment name. It also makes serial devices writable by everyone and terminates processes by name. The original file is preserved as evidence; the new deployment templates do not repeat those practices. Device accounts and passwords from the original material are not included in this guide or the templates.

## Network, serial, and video parameters

| Parameter | Original value | Meaning |
| --- | --- | --- |
| `UDP_RECEIVE_PORT` | `12345` | Binds all interfaces and receives PC gamepad UDP datagrams |
| `UDP_SEND_IP` / `UDP_SEND_PORT` | `192.168.0.1:54321` | Destination for forwarded controller serial data |
| `SERIAL_PORT` | `/dev/ttyTHS1` | Orin NX carrier-board UART; verify pins against the particular carrier revision |
| `BAUDRATE` | `115200` | Serial baud rate |
| Serial format | `8N1` | 8 data bits, no parity, 1 stop bit; XON/XOFF, RTS/CTS, and DSR/DTR flow control disabled |
| Serial read timeout | `0.1 s` | `serial.Serial(timeout=0.1)` |
| `VIDEO_DEVICE` | `/dev/video0` | USB camera device |
| `CAMERA_MODE` | `mjpeg` | Default MJPEG input |
| Video size / frame rate | `1280 × 480 @ 30 FPS` | Must match a mode supported by the actual camera |
| `VIDEO_BITRATE` | `4,000,000 bit/s` | NVENC target bitrate |
| `FORCE_TARGET_IP` / `PC_VIDEO_PORT` | `192.168.0.1:5600` | RTP/H.264 video destination |
| `RTP_PT` / `USE_NVENC` | `96` / `True` | RTP payload type; prefer the NVIDIA encoder |

These parameters are constants in the source. The original scripts have no command-line argument parser and do not automatically read environment-variable overrides. Private-network IPs in this table are retained deployment examples.

The wiring material requires the Orin to connect to RM-A UART3, with TX/RX crossed, a common ground, and the 5 V supply wire removed from this serial cable. Follow the actual carrier-board manual and wire order. This UART3 / RM-A arrangement must not be copied onto the V4 RM-C wiring.

## UDP gamepad format: 41 bytes

`JoystickState.from_bytes()` uses `struct.unpack('6f15B2b', data)`. The main loop accepts only datagrams exactly 41 bytes long.

| UDP offset | Type | Content |
| --- | --- | --- |
| 0–23 | `6f` | Six 32-bit floating-point gamepad axes |
| 24–38 | `15B` | Fifteen unsigned 8-bit button values |
| 39–40 | `2b` | Two signed 8-bit hat values |

**The source has no explicit byte-order prefix.** Both `6f15B2b` and the `5f3B2b` format below rely on Python `struct` native byte-order/alignment rules. They must not be described as an architecture-independent network protocol. The checking environment was little-endian and calculated sizes of 41 / 25 bytes respectively; the sender must match the target Jetson's actual layout.

## Downlink serial format: 28 bytes

The actual transmitted sequence is:

```text
0xAA | 25 data bytes (5f3B2b) | CRC16-MODBUS (2 bytes, little-endian)
```

**There is no length field and no `0xBB` terminator.** The CRC covers `0xAA + 25 data bytes`, with initial value `0xFFFF` and reflected polynomial `0xA001`. The result is written using `struct.pack('<H', crc)`.

| Data offset, excluding header | Type | Field | Gamepad calculation |
| --- | --- | --- | --- |
| 0–3 | float32 | `yaw_rate` | `-axes[0]` |
| 4–7 | float32 | `forward_speed` | `-axes[3]` |
| 8–11 | float32 | `left_right_speed` | `-axes[2]` |
| 12–15 | float32 | `down_speed` | `clamp((axes[4]+1)/2, 0, 1)` |
| 16–19 | float32 | `up_speed` | `clamp((axes[5]+1)/2, 0, 1)` |
| 20 | uint8 | `start_button` | `buttons[6]` |
| 21 | uint8 | `cam_fill_light` | `buttons[0]` |
| 22 | uint8 | `frame_fill_light` | `buttons[1]` |
| 23 | int8 | `cam_yaw` | `hat[0]` |
| 24 | int8 | `cam_pitch` | `hat[1]` |

Uplink telemetry is neither reassembled nor checked by this script. Each chunk returned by `ser.read(ser.in_waiting or 1)` directly becomes one UDP datagram. A serial-read chunk is not guaranteed to equal a complete firmware message; the PC receiver must reassemble data according to the actual protocol.

The corresponding imported firmware implementation is [`firmware/v4-pro/Services/V5Streamer/Streamer.hpp`](../../firmware/v4-pro/Services/V5Streamer/Streamer.hpp), whose frame structure matches this attachment. The UART3 control link is 115200 8N1. The firmware's IMU uses UART7 / 921600 8N1, which is a separate link and must not be confused with it. This establishes source-code correspondence, not a hardware integration result.

## Deployment preparation and template limits

The PC may use the GStreamer receive command from the original Orin guide. First ensure that Jetson's destination IP is this PC, both ends use port 5600, and the required GStreamer plugins are installed on the PC:

```sh
gst-launch-1.0 -v udpsrc port=5600 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink
```

This is a video-reception entry point. It does not include the matching Windows gamepad sender. Neither streaming nor reception was executed during this preparation.

The only additional Python dependency is `pyserial`. GStreamer, NVIDIA codec plugins, camera drivers, and serial permissions belong to the Jetson / JetPack system environment; neither `pip install serial` nor installing this requirements file alone replaces them.

A suggested layout is a project virtual environment at `/opt/iwin-v4/venv` and reviewed deployment copies at `/opt/iwin-v4/runtime/`. For example, after installing Python venv support on the target Jetson and preparing the target directories:

```bash
python3 -m venv /opt/iwin-v4/venv
/opt/iwin-v4/venv/bin/python -m pip install -r /opt/iwin-v4/source/software/v4-pro-jetson/requirements.txt
```

Both example services use `User=robot` and `Group=robot`; the deployer must create this user/group first. This is an example account name, not the original device account. The serial service adds the `dialout` group, and the video service adds `video`. If actual device nodes use different groups, adjust the templates accordingly and manage persistent permissions through the device's udev rules. Services use the virtual environment's Python directly and do not depend on interactive `conda activate`.

The templates' `ExecStart` points to deployment copies in `runtime/`; **they do not directly run `upstream/StreamerStart.sh`**. Before installation, verify device/firmware protocol compatibility and address the known limitations below, then place the two reviewed programs at the specified paths. The templates are not installed or enabled by default. No local or vehicle systemd configuration was changed during repository preparation.

Example deployment commands after preparation:

```bash
sudo install -m 0644 deploy/iwin-v4-pro-control.service.example /etc/systemd/system/iwin-v4-pro-control.service
sudo install -m 0644 deploy/iwin-v4-pro-video.service.example /etc/systemd/system/iwin-v4-pro-video.service
sudo systemctl daemon-reload
sudo systemctl start iwin-v4-pro-control.service
sudo systemctl start iwin-v4-pro-video.service
journalctl -u iwin-v4-pro-control.service -u iwin-v4-pro-video.service -f
```

The `start` commands above perform real actions: opening the serial port, listening for control traffic, and starting video. They are not offline checks and were not executed here. The deployer should decide whether to enable boot-time startup only after hardware acceptance. The services are managed independently; systemd cleans up their processes by service cgroup rather than global process-name matching.

## Known limitations: source preserved, not repaired

1. The `raw` branch in `GStreamerTest.py` lacks a final `!` before concatenating the encoder. Offline token construction confirmed the missing link operator; this mode cannot be treated as a validated pipeline.
2. With `USE_NVENC=False`, the preceding stage still outputs `video/x-raw(memory:NVMM)` before `videoconvert ! x264enc`. There is no explicit conversion back to system memory. The CPU fallback has a caps/memory-domain compatibility issue that requires verification and adjustment on the target JetPack installation.
3. Comments describing profile / GOP do not fully match the actual parameters. The code sets no profile, and `iframeinterval` is actually `int(VIDEO_FPS/2)`, which defaults to 15.
4. `VideoStreamer.stop()` catches Python's built-in `TimeoutError`, but `Popen.wait(timeout=5)` raises `subprocess.TimeoutExpired` on timeout. Verify the cleanup exception path in the deployment copy. The systemd template's cgroup management does not replace correct application error handling.
5. `V5StreamerNX.py` opens the serial port, creates sockets, and enters its loop at module scope. **Do not import the module directly as a side-effect-free check.** There is no reopen mechanism after the initial serial-open failure.
6. Any received 41-byte UDP payload is decoded as a gamepad packet, without sender authentication, sequence numbers, or rejection of expired commands. A UDP timeout only resumes waiting; it does not send zero output to the MCU. Disconnection stopping behavior must be confirmed in the target firmware and through end-to-end validation, not inferred from this bridge script.
7. Uplink `serial_receive_count` is not incremented at reception, so its statistics are not a reliable received-packet count.
8. If the GStreamer subprocess exits abnormally, the Python parent may still exit successfully. The template's `Restart=on-failure` is not a video-health check. If serial opening fails, the process may continue listening; systemd `active` status alone does not establish a working link.

Static syntax checks and isolated offline protocol checks were completed. Only pure function/class definitions were extracted, with output captured by an in-memory fake serial object. The checks confirmed UDP length 41, payload length 25, frame length 28, field mappings, and CRC test vectors. The serial script's module-level code was not imported, no serial port/socket was opened, and GStreamer was not started. These results do not establish hardware operability.

The checking environment was little-endian Python 3.14.5. Both Python attachments passed static compilation. The CRC16-MODBUS test vector `123456789` produced `0x4B37`. GStreamer commands were constructed only offline to confirm the raw-branch operator and CPU-branch NVMM limitations above; the constructed commands were not executed.

Original attachment SHA-256 hashes:

| File | SHA-256 |
| --- | --- |
| `V5StreamerNX.py` | `e6c97e906045df177d76d8b16685ee7cb6ad5dc434f04c072e52cb54ce6569b0` |
| `GStreamerTest.py` | `f00f7aed7b12b13e29c6668169819a32d81924ad64fccd49fb0bfee7a4da3692` |
| `StreamerStart.sh` | `bee9397ebb0cbe5d3f97b6cb76b76358e45ef7776723e313d4c86465845b168f` |
