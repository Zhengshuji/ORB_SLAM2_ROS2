cmake_minimum_required(VERSION 3.8)
project(orb_slam2_ros2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(message_filters REQUIRED)
# 第三方库依赖
find_package(OpenCV 4.0 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Pangolin REQUIRED)

# Thirdparty
message(STATUS "Thirdparty dir: ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty")
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty)

include_directories(
  include
  include/ORB_SLAM2
  ${EIGEN3_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${Pangolin_INCLUDE_DIRS}
  ${THIRD_PARTY_DIR}/DBoW2
  ${THIRD_PARTY_DIR}/DBoW2/DUtils
  ${THIRD_PARTY_DIR}/DBoW2/DBoW2
  ${THIRD_PARTY_DIR}/g2o
  ${THIRD_PARTY_DIR}/g2o/core
  ${THIRD_PARTY_DIR}/g2o/solvers
  ${THIRD_PARTY_DIR}/g2o/stuff
  ${THIRD_PARTY_DIR}/g2o/types
)

# 编译 DBoW2（指定编译类型为STATIC，避免动态库冲突）
add_subdirectory(${THIRD_PARTY_DIR}/DBoW2 EXCLUDE_FROM_ALL)
# 编译 g2o
add_subdirectory(${THIRD_PARTY_DIR}/g2o EXCLUDE_FROM_ALL)
# ORB-SLAM2 核心库
file(GLOB ORB_SLAM2_CORE_SRC
  "src/ORB_SLAM2/*.cc"
)
# 新增：验证源文件是否找到
message(STATUS "ORB_SLAM2 core files: ${ORB_SLAM2_CORE_SRC}")
add_library(orb_slam2_core SHARED
  ${ORB_SLAM2_CORE_SRC}
)
# 新增：指定库的链接顺序
target_link_libraries(orb_slam2_core
  PRIVATE
  ${OpenCV_LIBS}
  ${Pangolin_LIBRARIES}
  DBoW2
  g2o
)
# 新增：设置库的包含目录（传递给依赖的目标）
target_include_directories(orb_slam2_core
  PUBLIC
  include
  include/ORB_SLAM2
  ${EIGEN3_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${Pangolin_INCLUDE_DIRS}
  ${THIRD_PARTY_DIR}/DBoW2
  ${THIRD_PARTY_DIR}/g2o
)



add_executable(ros2_stereo_node src/ros2_stereo_node.cpp)
target_include_directories(ros2_stereo_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(ros2_stereo_node PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  ros2_stereo_node
  rclcpp
  sensor_msgs
  cv_bridge
  image_transport
  message_filters
)

# 链接核心库
target_link_libraries(ros2_stereo_node
  orb_slam2_core
  ${OpenCV_LIBS}
  ${Pangolin_LIBRARIES}
  DBoW2
  g2o
)

install(TARGETS ros2_stereo_node
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
