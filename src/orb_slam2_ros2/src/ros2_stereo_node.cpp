#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <memory>

// ROS2核心头文件
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "message_filters/subscriber.h"
#include "message_filters/sync_policies/approximate_time.h"
#include "message_filters/synchronizer.h"

// OpenCV头文件
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

// ORB-SLAM2核心头文件
#include "ORB_SLAM2/System.h"

using namespace std;
using namespace message_filters;

class ImageGrabber
{
public:
    ImageGrabber(ORB_SLAM2::System* pSLAM) : mpSLAM(pSLAM), do_rectify(false) {}

    void GrabStereo(const sensor_msgs::msg::Image::ConstSharedPtr& msgLeft,
                    const sensor_msgs::msg::Image::ConstSharedPtr& msgRight);

    ORB_SLAM2::System* mpSLAM;
    bool do_rectify;
    cv::Mat M1l, M2l, M1r, M2r;
};

int main(int argc, char **argv)
{
    // 初始化ROS2节点
    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("orb_slam2_stereo_node");

    // 参数检查
    if (argc != 4)
    {
        RCLCPP_ERROR(node->get_logger(), 
            "Usage: ros2 run orb_slam2_stereo stereo_node path_to_vocabulary path_to_settings do_rectify");
        rclcpp::shutdown();
        return 1;
    }

    // 创建ORB-SLAM2系统（双目模式）
    ORB_SLAM2::System SLAM(argv[1], argv[2], ORB_SLAM2::System::STEREO, true);
    ImageGrabber igb(&SLAM);

    // 解析是否在线校正
    stringstream ss(argv[3]);
    ss >> boolalpha >> igb.do_rectify;

    // 加载双目校正参数（如果需要）
    if (igb.do_rectify)
    {
        cv::FileStorage fsSettings(argv[2], cv::FileStorage::READ);
        if (!fsSettings.isOpened())
        {
            RCLCPP_ERROR(node->get_logger(), "ERROR: Wrong path to settings file!");
            rclcpp::shutdown();
            return -1;
        }

        cv::Mat K_l, K_r, P_l, P_r, R_l, R_r, D_l, D_r;
        fsSettings["LEFT.K"] >> K_l;
        fsSettings["RIGHT.K"] >> K_r;
        fsSettings["LEFT.P"] >> P_l;
        fsSettings["RIGHT.P"] >> P_r;
        fsSettings["LEFT.R"] >> R_l;
        fsSettings["RIGHT.R"] >> R_r;
        fsSettings["LEFT.D"] >> D_l;
        fsSettings["RIGHT.D"] >> D_r;

        int rows_l = fsSettings["LEFT.height"];
        int cols_l = fsSettings["LEFT.width"];
        int rows_r = fsSettings["RIGHT.height"];
        int cols_r = fsSettings["RIGHT.width"];

        if (K_l.empty() || K_r.empty() || P_l.empty() || P_r.empty() || R_l.empty() || R_r.empty() ||
            D_l.empty() || D_r.empty() || rows_l == 0 || rows_r == 0 || cols_l == 0 || cols_r == 0)
        {
            RCLCPP_ERROR(node->get_logger(), "ERROR: Calibration parameters missing!");
            rclcpp::shutdown();
            return -1;
        }

        // 计算校正映射
        cv::initUndistortRectifyMap(K_l, D_l, R_l, P_l.rowRange(0, 3).colRange(0, 3),
                                    cv::Size(cols_l, rows_l), CV_32F, igb.M1l, igb.M2l);
        cv::initUndistortRectifyMap(K_r, D_r, R_r, P_r.rowRange(0, 3).colRange(0, 3),
                                    cv::Size(cols_r, rows_r), CV_32F, igb.M1r, igb.M2r);
    }

    // 消息订阅（双目图像）
    message_filters::Subscriber<sensor_msgs::msg::Image> left_sub(node, "/camera/left/image_raw", rmw_qos_profile_default);
    message_filters::Subscriber<sensor_msgs::msg::Image> right_sub(node, "/camera/right/image_raw", rmw_qos_profile_default);

    // 时间同步策略（近似时间同步）
    typedef sync_policies::ApproximateTime<sensor_msgs::msg::Image, sensor_msgs::msg::Image> SyncPolicy;
    Synchronizer<SyncPolicy> sync(SyncPolicy(10), left_sub, right_sub);
    sync.registerCallback(std::bind(&ImageGrabber::GrabStereo, &igb, std::placeholders::_1, std::placeholders::_2));

    // 运行节点
    rclcpp::spin(node);

    // 停止SLAM系统
    SLAM.Shutdown();

    // 保存轨迹
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory_TUM_Format.txt");
    SLAM.SaveTrajectoryTUM("FrameTrajectory_TUM_Format.txt");
    SLAM.SaveTrajectoryKITTI("FrameTrajectory_KITTI_Format.txt");

    // 关闭ROS2
    rclcpp::shutdown();
    return 0;
}

void ImageGrabber::GrabStereo(const sensor_msgs::msg::Image::ConstSharedPtr& msgLeft,
                              const sensor_msgs::msg::Image::ConstSharedPtr& msgRight)
{
    // ROS2图像转OpenCV Mat
    cv_bridge::CvImageConstPtr cv_ptrLeft;
    try
    {
        cv_ptrLeft = cv_bridge::toCvShare(msgLeft);
    }
    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(rclcpp::get_logger("ImageGrabber"), "cv_bridge exception: %s", e.what());
        return;
    }

    cv_bridge::CvImageConstPtr cv_ptrRight;
    try
    {
        cv_ptrRight = cv_bridge::toCvShare(msgRight);
    }
    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(rclcpp::get_logger("ImageGrabber"), "cv_bridge exception: %s", e.what());
        return;
    }

    // 图像校正（如果需要）
    if (do_rectify)
    {
        cv::Mat imLeft, imRight;
        cv::remap(cv_ptrLeft->image, imLeft, M1l, M2l, cv::INTER_LINEAR);
        cv::remap(cv_ptrRight->image, imRight, M1r, M2r, cv::INTER_LINEAR);
        mpSLAM->TrackStereo(imLeft, imRight, cv_ptrLeft->header.stamp.sec + cv_ptrLeft->header.stamp.nanosec * 1e-9);
    }
    else
    {
        mpSLAM->TrackStereo(cv_ptrLeft->image, cv_ptrRight->image, cv_ptrLeft->header.stamp.sec + cv_ptrLeft->header.stamp.nanosec * 1e-9);
    }
}
