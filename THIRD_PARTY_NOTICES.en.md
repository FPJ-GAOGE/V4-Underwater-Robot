# Licenses, attribution, and sources

[中文](THIRD_PARTY_NOTICES.md) · **English**

This repository was organized at the project owner's request as a **private repository in preparation for an open-source release**. Original upstream licenses and copyright notices are preserved. No repository-wide MIT or GPL license has been added, and private collection does not establish public redistribution permission for all material.

| Scope | Source license or status | Treatment in this repository |
| --- | --- | --- |
| IWIN-FINS project in `firmware/v4` | Upstream [LICENSE](firmware/v4/LICENSE) contains the GNU GPL v3 text | Preserved unchanged; individual subcomponents retain their own terms |
| V4 CMSIS, ST HAL, and USB libraries | CMSIS: Apache 2.0; original HAL notices refer to BSD-3-Clause; USB includes ST SLA0044 | Corresponding license files and source headers are preserved |
| FineSUB in `firmware/v4-pro` | Upstream [LICENSE](firmware/v4-pro/LICENSE) is MPL-2.0 | File-level terms, attribution, and pinned dependency sources are preserved |
| FineSUB CMSIS-DSP, ETL, BSP, and other dependencies | Licenses in the respective directories; see [import record](firmware/v4-pro/IMPORT.en.md) | Versions and license files are preserved; exclusions of material unrelated to the target build are recorded |
| `software/v4-topside` | No explicit license at the specified commit | Included privately with provenance; no new license added to upstream code |
| Jetson and historical communication attachments | No explicit license in the supplied Feishu attachments | Original files, hashes, and document sources are preserved |
| CAD, drawings, photographs, BOM, and rewritten documentation | Supplied material has no separate open-hardware or documentation license statement; some models depict purchased components | Organized by variant with sources; unrestricted publication of supplier models is not assumed |

For material without clear permission, the project rights holders need to confirm distribution scope and licensing before public release. This does not prevent the requested private repository. Newly written documentation neither grants third-party permission nor automatically changes the terms of the referenced material.

## Included and excluded material

- Upstream author and team attribution, copyright headers, licenses, fixed commits, and original code behavior are preserved.
- Mechanical archives retain original filenames and native assemblies. Exchange-format files are verified individually; no license conversion is claimed.
- Feishu comments, OAuth responses, device passwords, personal task assignments, travel details, and expense records are excluded.
- Unconfirmed paper results, simulation models, and Pro configurations from other repositories are not represented as measured performance of this project.

See [source records](docs/sources.en.md) for software and design provenance.
