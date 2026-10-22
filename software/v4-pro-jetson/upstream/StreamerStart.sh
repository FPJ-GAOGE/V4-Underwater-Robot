#!/bin/bash

# start_streamer_system.sh - 系统级启动脚本

# 等待系统服务完全启动，确保网络和驱动准备就绪
echo "等待系统初始化..."
sleep 15

# 设置conda路径
export PATH="/home/fins/miniconda3/bin:$PATH"

# 修改设备权限
chmod 777 /dev/ttyTHS1

# 初始化conda
source /home/fins/miniconda3/etc/profile.d/conda.sh

# 激活环境
conda activate streamer

# 检查环境是否激活成功
if [ $? -ne 0 ]; then
    echo "错误: 无法激活conda环境 'streamer'"
    exit 1
fi

# 进入工作目录
cd /home/fins/Desktop/demo

# ----------------------------------------------------
# 1. 启动 GStreamer 推流程序 (后台运行)
# ----------------------------------------------------
# 关键：使用 & 符号，让它在后台持续运行
STREAMER_SCRIPT="GStreamerTest.py" # <--- 请根据您的实际文件名修改这里

echo "启动 GStreamer 视频推流 (${STREAMER_SCRIPT}) 到后台..."
python3.8 "${STREAMER_SCRIPT}" &

# 可选：等待几秒，确保推流管道稳定启动
sleep 3

# ----------------------------------------------------
# 2. 启动您的主程序 (前台运行)
# ----------------------------------------------------
echo "启动主程序 V5StreamerNX.py..."
python3.8 V5StreamerNX.py

# 主程序退出后，记录日志
echo "V5StreamerNX.py 已退出，退出代码: $?"

# ----------------------------------------------------
# 3. (可选) 清理：如果主程序退出，终止后台推流进程
# ----------------------------------------------------
# 查找并终止后台的 GStreamer 推流进程
# 警告: 仅在您确定推流不需要独立于 V5StreamerNX.py 运行时才使用此清理。
pkill -f "${STREAMER_SCRIPT}"