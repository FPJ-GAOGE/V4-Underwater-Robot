# V4 PC topside software import record

[中文](IMPORT.md) | English

## Source and scope

- Upstream repository: <https://gitee.com/superkinkin/rov-v4-wired-joystick>
- Upstream branch: `master`
- Pinned commit: `a65bc514808d2d8de929f05d2f42f31ed5bc848c`
- Import date: 2026-10-22
- Applicable link: V4 PC gamepad client → Ethernet TCP → Raspberry Pi → RM-C controller.
- This import does not establish successful integration with the specified `V4-ly` firmware, and it is not suitable for directly driving the V4-pro Orin NX / RM-A link.

## License status

The pinned commit has no `LICENSE`, `COPYING`, SPDX identifier, or other explicit source-code license declaration. Its original `README.md` also does not authorize redistribution under MIT, Apache-2.0, or similar terms.

These files are included in a private open-source preparation repository at the project owner's request, with their original source and commit retained. No new license has been applied to them. Before public release, the project rights holder should confirm authorization and a final license for this code. Licenses covering other parts of the repository do not automatically cover these upstream files.

## File roles

| File | Runtime location and purpose |
| --- | --- |
| `pc_client.py` | PC: Pygame gamepad interface; receives line-oriented text from the controller and writes `recv_log.txt`; includes servo start/stop commands |
| `pc_client_plot.py` | PC: control interface with live plots, selected using number keys; it still sends control commands and is not a read-only telemetry viewer |
| `test.py` | PC: interactive TCP text-sending utility; neither an automated test nor a Raspberry Pi server |
| `stream.sdp` | PC: RTP/H.264 session description for FFplay reception |
| `README.md` | Original upstream usage guide, retained unchanged |
| `requirements.txt` | PC Python dependency list added during this import |

**The Raspberry Pi program is not included in this upstream commit.** The Feishu “V4 User Guide” says the main Raspberry Pi code resides in `joystick_control` and starts through systemd, but does not provide the service name, entry point, or script contents. Cloning this directory is therefore insufficient to recreate the Raspberry Pi service. Upstream also contains no environment YAML, firmware, camera streaming script, or systemd unit.

The associated materials reviewed also contain a historical TCP 12345 image-transfer example. It cannot serve as the current TCP 5000 `joystick_control` service. The current server source was not found in the referenced repositories or attachments obtained for this preparation, and no replacement server was invented in this directory.

## Minimal PC installation and startup

Run these commands from this directory. Upstream provides no Python/Pygame version constraints. Install into a separate virtual environment on the target PC, verify gamepad compatibility, and then record the versions used.

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
```

The clients configure addresses directly in their source files. Defaults for the three programs are listed below. They are original device-network examples and contain no login credentials.

| Setting | File | Default |
| --- | --- | --- |
| Raspberry Pi IP | `PI_IP` in `pc_client.py` and `pc_client_plot.py`; `RASPBERRY_IP` in `test.py` | `192.168.138.2` |
| Control connection | Corresponding `PI_PORT` / `RASPBERRY_PORT` | TCP `5000` |
| Example PC address | Feishu “V4 User Guide” | `192.168.138.10`; must share a subnet with the actual device |

After checking the device address, Raspberry Pi service, firmware protocol, and gamepad-axis mapping, run one client:

```powershell
.\.venv\Scripts\python.exe pc_client.py
# Or use the plotting control client; there is no need to run both together
.\.venv\Scripts\python.exe pc_client_plot.py
```

Both clients create a GUI and attempt a device connection at startup. Do not use `import pc_client` as a side-effect-free check: GUI initialization occurs at module scope. Manual text sending through `test.py` is not a substitute for the binary protocol below.

### Video reception

For the upstream default GStreamer approach, install GStreamer and the required H.264 plugins separately, and confirm that the Raspberry Pi sends to this PC's UDP port 5600:

```text
gst-launch-1.0 -v udpsrc port=5600 caps="application/x-rtp, media=video, encoding-name=H264, payload=96, clock-rate=90000" ! rtpjitterbuffer latency=10 drop-on-latency=true ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false
```

Upstream also provides an FFplay approach. `stream.sdp` uses receive address `192.168.138.10` and RTP port **5004**, which differs from **5600** in the GStreamer example. Adjust these for the actual sender; do not treat the defaults as equivalent:

```text
ffplay -fflags nobuffer -flags low_delay -sync video -framedrop -probesize 32 -analyzeduration 0 -protocol_whitelist file,udp,rtp,tcp,https,crypto -i stream.sdp
```

## Interfaces to verify against firmware

This directory implements only a TCP client and **does not configure a serial device name, baud rate, data bits, parity, or stop bits**. Serial settings cannot be inferred from TCP port 5000. Check the Raspberry Pi forwarding program together with `V4-ly` UART initialization. The Feishu V4 wiring material refers to RM-C UART6, but the connector location alone does not establish the software's baud rate or selected serial port.

The PC's `send_pkt` writes the following bytes directly into the TCP stream, without a length field, checksum, or terminator. If the Raspberry Pi assembles frames, that behavior must be established from its actual source; a single TCP `recv` is not guaranteed to contain one complete command.

| Function | PC byte sequence | Actual trigger |
| --- | --- | --- |
| Hover toggle | `02 10` | Button 7; local `isHovering` state toggles accordingly |
| Yaw closed-loop toggle | `02 20` | Button 0 |
| Horizontal thrust | `02 30` + `int8 x` + `int8 y` | Left stick; approximately -99 to 99 |
| Yaw angular velocity | `02 23` + little-endian `int16` | Right stick, horizontal |
| Depth-force input | `02 02` + little-endian `int16` | Right stick, vertical |
| Servo start / stop | `03 00` / `03 01` | Buttons 1 / 2 in `pc_client.py` only |

Existing behaviors that require individual verification:

1. While hovering, `pc_client.py` sends horizontal, yaw, and depth commands only when their values change. `pc_client_plot.py` sends them on every main-loop iteration, targeting 30 FPS. Their different send rates may affect firmware with communication-timeout behavior.
2. The ordinary client's heartbeat-sending code is commented out. The presence of a `HEARTBEAT_INTERVAL` constant does not establish an active periodic heartbeat.
3. On exit, the program only closes its socket; it does not send an explicit stop frame. Disconnection and stopping behavior depend on the Raspberry Pi and firmware. Upstream control logic was not changed during this import.
4. The GUI depth target and D-pad servo angle are not necessarily transmitted control values. `MOT:` servo-angle sending is commented out in the ordinary client, and the plotting client does not send that angle either.
5. `pc_client_plot.py` parses only newline-separated numeric text. Actual telemetry must match this format. Neither client may be assumed compatible with an unverified binary telemetry format.
6. Gamepad axes are fixed to IDs 0, 1, 2, and 3. The Feishu usage guide explicitly notes that mappings may differ between operating systems; verify the input values and their directions first.

These are static observations about the pinned source, not a claim of hardware-validated compatibility.

## Import cleanup and verification scope

- Exported original Git objects: `pc_client.py`, `pc_client_plot.py`, `test.py`, `stream.sdp`, and `README.md`. Original bytes were read using `git -c core.autocrlf=false show <commit>:<path>` at the pinned commit to avoid Windows checkout CRLF conversion. Program contents were not changed.
- Excluded: upstream `.git/` history and the empty `recv_log.txt`. No executables, virtual environments, runtime logs, or credentials were imported.
- No hardcoded passwords, access tokens, or private keys were found in the three upstream Python files or video configuration; there was no actual password to replace. Example device IPs remain to preserve the original configuration facts and should be adjusted for deployment.
- Added: this import record, a dependency list containing only `pygame`, and a local Python-oriented `.gitignore`. Upstream does not pin dependency versions, so no reproducible version lock is claimed here.
- Only offline static syntax parsing, dependency-import inventory checks, and SHA-256 comparison with upstream were performed. No client was started, no physical device was connected, and no control command was sent.
- The checking environment was Python 3.14.5. All three Python files passed AST parsing and static compilation. Git blob IDs were calculated as `blob <byte count>\0<content>` for all five source files and matched the object IDs in the pinned upstream commit; SHA-256 hashes were also recorded. `pygame` was not installed in the checking environment, so GUI execution and dependency installation were not validated.
- Passing Python static syntax checks only means the files can be parsed. It does not validate the GUI, gamepad, network, video, or the link to `V4-ly` firmware.
