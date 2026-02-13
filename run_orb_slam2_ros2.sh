#!/bin/bash

# ==============================================================================
# ORB_SLAM2 ROS2 双目节点运行脚本
# 功能：通过变量管理参数，一键运行ros2_stereo_node节点
# 使用说明：修改下方的变量为你的实际路径后，执行 chmod +x run_stereo_orb_slam2.sh && ./run_stereo_orb_slam2.sh
# ==============================================================================

# -------------------------- 核心参数配置（请根据实际路径修改） --------------------------
# ORB词典文件路径（ORBvoc.txt）
VOCAB_PATH="./src/orb_slam2_ros2/Vocabulary/ORBvoc.txt"

# 双目相机配置文件路径（stereo.yaml）
CONFIG_PATH="./src/orb_slam2_ros2/config/stereo.yaml"

# 是否校正图像（0=不校正，1=校正，根据相机实际情况选择）
DO_RECTIFY=1

# -------------------------- 前置检查（避免运行时出错） --------------------------
# 检查词典文件是否存在
if [ ! -f "$VOCAB_PATH" ]; then
    echo -e "\033[31m错误：词典文件不存在！路径：$VOCAB_PATH\033[0m"
    echo "请检查VOCAB_PATH变量是否配置正确，或执行以下命令查找文件："
    echo "find /home/zsj/WorkSpace/ORB2_SLAM/ORB_SLAM2_ROS2 -name \"ORBvoc.txt\""
    exit 1
fi

# 检查配置文件是否存在
if [ ! -f "$CONFIG_PATH" ]; then
    echo -e "\033[31m错误：配置文件不存在！路径：$CONFIG_PATH\033[0m"
    echo "请检查CONFIG_PATH变量是否配置正确，或执行以下命令查找文件："
    echo "find /home/zsj/WorkSpace/ORB2_SLAM/ORB_SLAM2_ROS2 -name \"*stereo*.yaml\""
    exit 1
fi

# 检查校正标志是否为0或1
if [ "$DO_RECTIFY" != "0" ] && [ "$DO_RECTIFY" != "1" ]; then
    echo -e "\033[31m错误：校正标志只能是0或1！当前值：$DO_RECTIFY\033[0m"
    exit 1
fi

# -------------------------- 打印运行信息（方便核对参数） --------------------------
echo -e "\033[32m===== 即将运行ORB_SLAM2 ROS2双目节点 =====\033[0m"
echo "词典文件路径：$VOCAB_PATH"
echo "配置文件路径：$CONFIG_PATH"
echo "是否校正图像：$DO_RECTIFY (0=否，1=是)"
echo -e "\033[32m==========================================\033[0m"
sleep 2  # 延迟2秒，方便你核对参数

# -------------------------- 核心运行指令（最后一行） --------------------------
ros2 run orb_slam2_ros2 ros2_stereo_node "$VOCAB_PATH" "$CONFIG_PATH" "$DO_RECTIFY"