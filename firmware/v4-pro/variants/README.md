# V4-pro 实例分支

父目录默认保存上游 `v4pro1@5a7832648206e522e9b8e0dff108638789f6afdd` 原始固件。此目录的 [v4-pro1-to-pro2.patch](v4-pro1-to-pro2.patch) 将 3 个固件文件精确转换到上游 `V4pro2@847ff0309c54016365c303b686369af9d005db41`。

两个名称均来自 [FineSUB 上游分支](https://gitee.com/gan-ju/FineSUB)。飞书制作资料没有指定 `pro1` 或 `pro2`，因此这只是可追溯的两套实例参考，不代表已经确认用户哪台潜器对应哪个分支。

补丁涵盖：

- `Interface/Examples/TaskSUB.cpp`：DSHOT ID 与电机位置关系。
- `Devices/IMU/IMUBase.hpp`：IMU 安装补偿矩阵。
- `Components/SUB/V5_SUB.hpp`：推进器输出符号、水压计通道和 yaw 输入处理。

另一处上游 `.idea/runConfigurations/Robomaster_C.xml` 变化属于 IDE 运行配置，未导入。源码行为、数值和应用方法详见 [IMPORT.md](../IMPORT.md#v4-pro1--v4-pro2-差异)。

在**单独复制的固件目录**里执行：

```powershell
git init
git -c core.autocrlf=false apply --check variants/v4-pro1-to-pro2.patch
git -c core.autocrlf=false apply variants/v4-pro1-to-pro2.patch
```

不要将补丁重复应用，不要在已调整参数的工作副本上未经审查地覆盖。编译目标仍然为 RoboMaster A；补丁不把 A 板固件变成 C 板固件。本次已验证补丁可应用，施加后的 3 个文件与固定 pro2 提交逐字节相同。尚未完成 ARM 工具链编译或实机验证。
