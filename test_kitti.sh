#!/bin/bash
# run_stereo_kitti.sh - 独立运行stereo_kitti，不影响原有ROS2脚本

source install/setup.sh

# 1. 定义路径（根据你的实际路径调整）
PROJECT_ROOT=$(cd $(dirname $0); pwd)
VOCAB_PATH="/home/zsj/WorkSpace/ORB2_SLAM/ORB_SLAM2_ROS2/src/orb_slam2_ros2/Vocabulary/ORBvoc.txt"
CONFIG_PATH="/home/zsj/WorkSpace/ORB2_SLAM/ORB_SLAM2_ROS2/src/orb_slam2_ros2/config/KITTI00-02.yaml"  # KITTI配置文件
KITTI_SEQUENCE_PATH="${PROJECT_ROOT}/src/data_odometry_gray/dataset/sequences/00"  # 你的KITTI数据集序列路径

## -------------------------- （动态）库路径添加 --------------------------
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./build/orb_slam2_ros2/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./src/orb_slam2_ros2/Thirdparty/DBoW2/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./src/orb_slam2_ros2/Thirdparty/g2o/lib/

export ASAN_OPTIONS=new_delete_type_mismatch=0

echo -e "\033[32m===== 即将运行ORB_SLAM2 ROS2 KITTI节点 =====\033[0m"
echo "词典文件路径：$VOCAB_PATH"
echo "配置文件路径：$CONFIG_PATH"
echo "KITTI序列路径：$KITTI_SEQUENCE_PATH"
echo -e "\033[32m==========================================\033[0m"
sleep 2  # 延迟2秒，方便你核对参数

# 4. 运行stereo_kitti（参数与原逻辑一致）
#ros2 run orb_slam2_ros2 stereo_kitti_node
ros2 run orb_slam2_ros2 stereo_kitti_node "$VOCAB_PATH" "$CONFIG_PATH" "$KITTI_SEQUENCE_PATH"