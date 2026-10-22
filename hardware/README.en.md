# Mechanical and Electrical Files

[中文](README.md) | English

CAD for the two models is stored separately. Identical filenames do not establish identical geometry or mounting interfaces.

| Directory / original archive | Revision and contents |
| --- | --- |
| `v4/cad/` | STEP/DWG files from the V4 mechanical table's 2025-05-23 “refined design” revision; 21 files |
| `v4/camera-mount/` | Monocular camera mount dated 2025-07-10; two SolidWorks parts |
| `v4/manipulator/` | Manipulator design attached to the design document; seven parts and one assembly file, separate from the current control chain |
| `v4/electrical-connection.original.pdf` | Original one-page electrical drawing, retaining historical design annotations; see the corresponding hardware guide for discrepancies |
| `v4-pro/cad/` | STEP/STP, DWG, and `thrust.m` extracted from the original V4pro archive; 35 files |

## Complete native CAD

[Download the complete original archives from the private release](https://github.com/FPJ-GAOGE/V4-Underwater-Robot/releases/tag/mechanical-source-2026-10-22):

- `V4-2025-05-23.zip`: 83,275,936 bytes; original filename `V4.zip`.
- `V4pro.zip`: 348,312,112 bytes.
- `v4-camera-mount-2025-07-10.zip`: 275,425 bytes; original filename `相机支架.zip`.
- `v4-manipulator.zip`: 1,530,494 bytes; original filename `机械爪结构.zip`.

The original archives preserve complete SolidWorks part, assembly, and drawing relationships. The neutral formats extracted into Git do not replace the complete assembly projects. V4 and Pro archive filenames use GBK encoding. If Chinese names appear corrupted after extraction, use a tool that supports GBK/CP936. Extracted Git paths have already been decoded correctly into Chinese.

SHA-256 checksums for the archives, every file inside them, and the files extracted into Git are recorded in [assets-manifest.json](assets-manifest.json). A concise checksum list for the published archives is available in [archives.sha256](archives.sha256). `repository_path=null` means that the file is available only in the complete original archive. Original SolidWorks temporary lock files are also listed in the archive manifest but are not included in the neutral-format directories.

## Reading order

Read the [model comparison](../docs/variants.en.md), then the [BOM](../docs/bom.en.md), followed by [V4 assembly and wiring](../docs/hardware-v4.en.md) or [V4-pro assembly and wiring](../docs/hardware-v4-pro.en.md). Before manufacturing, check materials, dimensions, threads, and the actual purchased components in CAD. Archive-integrity checks do not establish manufacturability or pressure qualification.

The mechanical and photographic materials do not include a separate open-hardware license. See [third-party notices](../THIRD_PARTY_NOTICES.en.md).
