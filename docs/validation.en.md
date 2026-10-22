# Verification record for this repository preparation

[中文](validation.md) | English

Verification date: 2026-10-22. Scope: documentation, source snapshots, and offline protocols; no hardware operation is included.

## Completed checks

| Item | Check and evidence |
| --- | --- |
| Three user-specified Feishu documents | Full text read through the SJTU account; main-document revision IDs and directly related sources are listed in [sources.en.md](sources.en.md) |
| V4 control source | All 347 imported files checked against the pinned commit using SHA-256 and Git blob IDs; [UPSTREAM.json](../firmware/v4/UPSTREAM.json) |
| V4-pro control source and dependencies | All 1,658 source files checked against pinned Git objects, totaling 49,356,433 bytes; 10 nested IDE files excluded and 10 ETL generator scripts retained; [UPSTREAM.json](../firmware/v4-pro/UPSTREAM.json) |
| pro1 → pro2 patch | Passed `git apply --check` in an independent clean copy; all 3 patched files matched the target commit byte for byte |
| V4 PC source | Original 5 files checked against Git objects using `git show`, preserving original line endings; [UPSTREAM.json](../software/v4-topside/UPSTREAM.json) |
| CAD | 4 original archives, containing 275 non-directory entries; 66 files extracted into Git and checked individually by SHA-256; Chinese filenames decoded using their original encoding |
| PDF and images | Original electrical PDF contains 1 page and was rendered and inspected; simplified wiring diagram assigned to Pro based on its actual A-board content; voltage conflicts between diagram labels and prose are documented |
| Python syntax | All 12 Python source files under software passed static compilation; their module-level runtime code was not imported |
| Jetson control protocol | Pure definitions extracted from AST only; in-memory fake serial output used to verify 41-byte gamepad input, 25-byte payload, 28-byte serial frame, field directions, and CRC; CRC16/Modbus for `123456789` was `0x4B37` |
| Source documentation | Independent source review added the H30 dependency and clarified DSHOT/PWM differences |
| Privacy and cleanup | GitHub repository set to private; raw Feishu responses and authentication material remain in ignored directories; device passwords, comments, and personnel/expense records are not committed |

## Bilingual documentation and BOM checks

All 20 authored document pairs have full Chinese and English versions with language navigation. Technical numbers, units, protocols, variant differences, and local file links have been checked. The bilingual home page follows the FinsROV reference presentation, with distinct BOM, mechanical, controller, and deployment entry points.

The bilingual [Excel BOM](../hardware/bom/V4-V4pro-BOM.xlsx) contains 4 worksheets: V4 Chinese / English and V4-pro Chinese / English. It covers 95 records (56 for V4 and 39 for V4-pro), with 47 unknown quantities retained as blank cells. Every Excel and CSV record has been reconciled with the JSON source dataset, including names, quantities, units, specifications, statuses, and provenance. All four sheets contain filterable tables and frozen panes; 66 supplier hyperlinks match the source records. Every sheet has been rendered and visually inspected. No prices or aggregate purchase cost have been invented.

The BOM covers the available parts list, build instructions, historical alternatives, and identifiable mechanical parts. It does not claim a complete assembly-instance count. Historical alternatives, optional design files, and unknown quantities are labeled separately; the 95 rows are not a purchase order for one vehicle.

## Repeatable offline checks

From the repository root, using Python 3.10 or newer:

```sh
python tools/validate_repository.py
```

This checks source manifests, extracted CAD and attachment SHA-256 hashes, Python syntax, bilingual navigation and local links, record-level BOM consistency across JSON/CSV/Excel and supplier hyperlinks, and in-memory protocol encoding. It does not execute the original scripts' serial/socket initialization, connect to a vehicle, or start GStreamer.

After downloading original archives, also check their SHA-256 hashes against [archives.sha256](../hardware/archives.sha256). A passing working-directory check alone does not prove that Release attachments were transferred intact. Import hashes establish provenance consistency, not absence of defects in the original programs.

## Checks not completed or claimed

- No usable firmware cross-compilation toolchain was available locally. V4 and Pro target compilation, linking, flashing, and runtime validation were not completed. Pro's MDK project still omits TaskSUB.cpp; the mere existence of a project file does not prove buildability.
- The complete Pygame, PySerial, JetPack, camera, and GStreamer environments were not installed or integrated.
- No vehicle was connected. ESC neutral behavior, DSHOT mode, rotation direction, closed-loop stability, stopping on disconnection, leak-tightness, pressure resistance, endurance, and maximum speed were not tested.
- Missing V4 Raspberry Pi server and Pro Windows client programs were not invented as replacements; no new license declarations were added to material with unclear licensing.

Specific limitations are retained in [known-limitations.en.md](known-limitations.en.md).
