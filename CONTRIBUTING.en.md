# Contributing

[中文](CONTRIBUTING.md) · **English**

Identify the applicable variant in each change: V4, V4-pro1, V4-pro2, or material shared by both platforms. Preserve source provenance and copyright notices.

## Hardware and control parameters

Record board, thruster, and ESC models, IMU mounting orientation, pressure sensor positions, firmware commit, and validation scope. Explain thruster channels, propeller directions, thrust allocation, sensor coordinate systems, and control gains together. Changing only a sign or version flag is not enough to describe a hardware change.

Do not present static compilation, offline tests, or simulation as hardware validation. For physical tests, include sanitized conditions, results, failure observations, and logs. Do not commit credentials or personal device configuration.

## Documentation and models

Link technical conclusions to pinned source code or specific design versions where possible. When sources conflict, record both statements and their applicable dates instead of silently choosing one. Store complete CAD archives in Releases and keep exchange formats and SHA-256 manifests in Git.

Maintain Chinese documentation and its `.en.md` English counterpart together. Quantities, units, protocol fields, and limitations must agree. Bilingual BOM source data is stored in `hardware/bom/bom-data.json`. When it changes, update both variant CSV files and the Excel workbook, preserving stable item IDs, sources, and explanations for unknown quantities. Do not turn an unknown quantity into zero.

The repository's `tools/validate_repository.py` checks links, CAD integrity, and Python syntax. It does not replace firmware builds or hardware tests. Run it from the repository root before submitting:

```sh
python tools/validate_repository.py
```

Keep imported upstream code separate. Functional fixes should be clear, independent commits describing their differences from upstream. Before adding a missing Raspberry Pi or Windows program, verify its protocol and license, then place it in the appropriate variant directory.
