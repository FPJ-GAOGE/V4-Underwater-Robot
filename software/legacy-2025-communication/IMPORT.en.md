# Historical communication example import record

[中文](IMPORT.md) | English

This directory preserves an early TCP image-communication example from the 2025 V4 development material. **It is neither the current V4 Raspberry Pi `joystick_control` service nor the V4-pro UDP / UART bridge.**

The source is `wirelesscommunicator.zip`, an attachment associated with the Feishu document [PC and Raspberry Pi Communication](https://iwin-fins.feishu.cn/wiki/YMpxw929tiODo1kBmVRcYpy4noh). Import date: 2026-10-22. The attachment has neither a Git commit ID nor an explicit license; MIT or another open-source authorization must not be assumed. Confirm the original authors' permission as part of the project before public release.

Only the archive's 7 `.py` files were extracted, retaining original filenames and bytes. `.idea/` project settings, device/test photographs, logs, and runtime environments were not imported. The spelling `main_sever.py` comes from the original file and was not changed. No passwords, access tokens, or private keys were found in the code. Private-network IPs remain as original historical configuration values.

| File | Historical purpose |
| --- | --- |
| `server.py`, `main_sever.py` | Raspberry Pi TCP image servers, opening camera 0 |
| `client.py`, `main_client.py` | PC image reception and display |
| `server_test.py`, `client_test.py` | Single-image TCP transfer attempts using different historical IPs |
| `camera test.py` | Local OpenCV preview test using camera 1 |

The main transport port is **TCP 12345**. The current V4 PC gamepad client uses **TCP 5000**, while V4-pro uses **UDP 12345**. Equal port numbers do not imply equal transport layers or protocols. This directory contains no MCU serial interface, gamepad forwarding, or implementation of the current `joystick_control` systemd service.

Historical dependencies are the Python standard library and OpenCV, imported as `cv2` and usually supplied by `opencv-python`. The original attachment provides no Python/OpenCV version constraints or dependency lock; compatibility with modern environments has not been verified.

## Known incomplete behavior

- Although `server.py` creates JPEG-encoded data and calls `pickle.dumps(buffer)`, it actually sends `len(image)` and `image`, not the generated `data`. This does not match the client's expected “4-byte network-order length + pickle payload” and cannot be presented as a working image-protocol example.
- After receiving a payload, `client.py` reads the socket again, and its local buffer cannot persistently retain the next frame. This can cause blocking or dropped frames. It also lacks complete EOF handling when a connection closes early.
- `client.py` deserializes network input with `pickle.loads`, which can execute code embedded in malicious pickle data. Do not run this historical implementation on untrusted network data.
- `server_test.py` depends on `image.jpg`, which was not included in this code import; photographs are outside this source-code deliverable.
- Several scripts open cameras, create/connect sockets, or enter loops at module scope. Do not import them directly for static checking.

These historical algorithms and protocols were not repaired. Only AST / compile static syntax checks were performed, together with byte-for-byte verification of all 7 files against the ZIP archive. No network, camera, or image program was started, and these examples were not integrated into the current V4/V4-pro runtime workflow.
