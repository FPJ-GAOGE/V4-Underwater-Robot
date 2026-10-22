# V4-pro instance branches

[中文](README.md) | English

The parent directory contains the original firmware from upstream `v4pro1@5a7832648206e522e9b8e0dff108638789f6afdd` by default. The [v4-pro1-to-pro2.patch](v4-pro1-to-pro2.patch) in this directory converts 3 firmware files exactly to upstream `V4pro2@847ff0309c54016365c303b686369af9d005db41`.

Both names come from the [FineSUB upstream branches](https://gitee.com/gan-ju/FineSUB). The Feishu manufacturing material does not specify `pro1` or `pro2`; these are two traceable instance references, not confirmation of which branch matches any particular vehicle owned by the user.

The patch covers:

- `Interface/Examples/TaskSUB.cpp`: DSHOT IDs and their relationship to motor positions.
- `Devices/IMU/IMUBase.hpp`: IMU mounting-compensation matrix.
- `Components/SUB/V5_SUB.hpp`: thruster-output signs, pressure channels, and yaw-input handling.

The additional upstream change to `.idea/runConfigurations/Robomaster_C.xml` is an IDE run configuration and was not imported. See [IMPORT.en.md](../IMPORT.en.md#v4-pro1--v4-pro2-differences) for source behavior, numeric values, and application details.

Run these commands inside a **separately copied firmware directory**:

```powershell
git init
git -c core.autocrlf=false apply --check variants/v4-pro1-to-pro2.patch
git -c core.autocrlf=false apply variants/v4-pro1-to-pro2.patch
```

Do not apply the patch twice or overwrite a parameter-adjusted working copy without review. The build target remains RoboMaster A; this patch does not convert A-board firmware into C-board firmware. Patch application was verified, and the resulting 3 files match the pinned pro2 commit byte for byte. ARM-toolchain compilation and hardware validation have not been completed.
