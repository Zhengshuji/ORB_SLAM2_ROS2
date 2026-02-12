cmake_minimum_required(VERSION 3.16)
project(orb_slam2_stereo)

# ROS2 Humble C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -march=native")

# 关闭 Eigen 旧接口警告
add_definitions(-DEIGEN_NO_DEPRECATED_WARNING)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(message_filters REQUIRED)

find_package(OpenCV 4.0 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Pangolin REQUIRED)

# ==============================================
# 关键修复：第三方库路径
# ==============================================
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty)

include_directories(
  include
  include/ORB_SLAM2

  ${EIGEN3_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${Pangolin_INCLUDE_DIRS}

  # 这里是真正能找到 DBoW2/g2o 头文件的路径
  ${THIRD_PARTY_DIR}/DBoW2
  ${THIRD_PARTY_DIR}/DBoW2/DUtils
  ${THIRD_PARTY_DIR}/DBoW2/DBoW2
  ${THIRD_PARTY_DIR}/g2o
  ${THIRD_PARTY_DIR}/g2o/core
  ${THIRD_PARTY_DIR}/g2o/solvers
  ${THIRD_PARTY_DIR}/g2o/stuff
  ${THIRD_PARTY_DIR}/g2o/types
)

# 编译 DBoW2
add_subdirectory(${THIRD_PARTY_DIR}/DBoW2)

# 编译 g2o
add_subdirectory(${THIRD_PARTY_DIR}/g2o)

# ORB-SLAM2 核心
file(GLOB ORB_SLAM2_CORE_SRC
  "src/ORB_SLAM2/*.cc"
)

add_library(orb_slam2_core SHARED
  ${ORB_SLAM2_CORE_SRC}
)

target_link_libraries(orb_slam2_core
  ${OpenCV_LIBS}
  ${Pangolin_LIBRARIES}
  DBoW2
  g2o
)

# ROS2 双目节点
add_executable(stereo_node
  ros2_stereo_node.cc
)

ament_target_dependencies(stereo_node
  rclcpp
  sensor_msgs
  cv_bridge
  image_transport
  message_filters
)

target_link_libraries(stereo_node
  orb_slam2_core
  ${OpenCV_LIBS}
  ${Pangolin_LIBRARIES}
  DBoW2
  g2o
)

# 安装
install(TARGETS
  stereo_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  config
  Vocabulary
  DESTINATION share/${PROJECT_NAME}
)

ament_package()