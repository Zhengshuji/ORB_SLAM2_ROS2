/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/

#include<iostream>
#include<algorithm>
#include<fstream>
#include<iomanip>
#include<chrono>

#include<opencv2/core/core.hpp>

#include<ORB_SLAM2/System.h>

using namespace std;

#define COMPILEDWITHC11 1

void LoadImages(const string &strPathToSequence, vector<string> &vstrImageLeft,
                vector<string> &vstrImageRight, vector<double> &vTimestamps);

int main(int argc, char **argv)
{
    if(argc != 4)
    {
        cerr << endl << "Usage: ./stereo_kitti path_to_vocabulary path_to_settings path_to_sequence" << endl;
        return 1;
    }

    // Retrieve paths to images
    vector<string> vstrImageLeft;
    vector<string> vstrImageRight;
    vector<double> vTimestamps;
    LoadImages(string(argv[3]), vstrImageLeft, vstrImageRight, vTimestamps);

    const int nImages = vstrImageLeft.size();

    // 检查图像列表是否为空
    if(nImages == 0)
    {
        cerr << "ERROR: No images loaded from sequence path: " << argv[3] << endl;
        return 1;
    }

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM2::System SLAM(argv[1],argv[2],ORB_SLAM2::System::STEREO,true);

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    vTimesTrack.resize(nImages);

    cout << endl << "-------" << endl;
    cout << "Start processing sequence ..." << endl;
    cout << "Images in the sequence: " << nImages << endl << endl;   

    // Main loop
    cv::Mat imLeft, imRight;
    bool bHasValidTrajectory = false; // 标记是否有有效轨迹
    for(int ni=0; ni<nImages; ni++)
    {
        // Read left and right images from file
        imLeft = cv::imread(vstrImageLeft[ni], cv::IMREAD_UNCHANGED);
        imRight = cv::imread(vstrImageRight[ni], cv::IMREAD_UNCHANGED);

        double tframe = vTimestamps[ni];

        if(imLeft.empty() || imRight.empty())
        {
            cerr << endl << "Failed to load image at: "
                 << (imLeft.empty() ? vstrImageLeft[ni] : vstrImageRight[ni]) << endl;
            continue; // 跳过无效帧，而非直接退出
        }

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
#endif

        // Pass the images to the SLAM system
        SLAM.TrackStereo(imLeft,imRight,tframe);
        bHasValidTrajectory = true; // 只要跟踪过一帧，标记为有轨迹

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
#endif

        double ttrack= std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();

        vTimesTrack[ni]=ttrack;

        // Wait to load the next frame
        double T=0;
        if(ni<nImages-1)
            T = vTimestamps[ni+1]-tframe;
        else if(ni>0)
            T = tframe-vTimestamps[ni-1];

        if(ttrack<T)
            usleep((T-ttrack)*1e6);

        cout << "\rProgress: " << (ni+1)*100/nImages << "%" << flush;
    }
    cout << endl; // 换行，避免进度条覆盖后续输出

    // ===== 修正顺序：先停止线程，再处理统计和保存 =====
    // Stop all threads FIRST
    SLAM.Shutdown();

    // Tracking time statistics
    sort(vTimesTrack.begin(),vTimesTrack.end());
    float totaltime = 0;
    int nValidTracks = 0;
    for(int ni=0; ni<nImages; ni++)
    {
        if(vTimesTrack[ni] > 0) // 只统计有效跟踪时间
        {
            totaltime += vTimesTrack[ni];
            nValidTracks++;
        }
    }
    cout << "-------" << endl << endl;
    if(nValidTracks > 0)
    {
        cout << "median tracking time: " << vTimesTrack[nImages/2] << endl;
        cout << "mean tracking time: " << totaltime/nValidTracks << endl;
    }
    else
    {
        cout << "No valid tracking time data" << endl;
    }

    // Save camera trajectory (仅当有有效轨迹时保存)
    if(bHasValidTrajectory)
    {
        // 检查文件写入权限：指定绝对路径，避免权限问题
        string strTrajectoryPath = "./CameraTrajectory.txt";
        cout << "Saving camera trajectory to " << strTrajectoryPath << " ..." << endl;
        SLAM.SaveTrajectoryKITTI(strTrajectoryPath);
        cout << "Trajectory saved successfully!" << endl;
    }
    else
    {
        cerr << "WARNING: No valid trajectory to save!" << endl;
    }

    return 0;
}

void LoadImages(const string &strPathToSequence, vector<string> &vstrImageLeft,
                vector<string> &vstrImageRight, vector<double> &vTimestamps)
{
    ifstream fTimes;
    string strPathTimeFile = strPathToSequence + "/times.txt";
    fTimes.open(strPathTimeFile.c_str());
    if(!fTimes.is_open())
    {
        cerr << "ERROR: Cannot open times.txt at " << strPathTimeFile << endl;
        return;
    }
    while(!fTimes.eof())
    {
        string s;
        getline(fTimes,s);
        if(!s.empty())
        {
            stringstream ss;
            ss << s;
            double t;
            ss >> t;
            vTimestamps.push_back(t);
        }
    }
    fTimes.close(); // 显式关闭文件

    string strPrefixLeft = strPathToSequence + "/image_0/";
    string strPrefixRight = strPathToSequence + "/image_1/";

    const int nTimes = vTimestamps.size();
    vstrImageLeft.resize(nTimes);
    vstrImageRight.resize(nTimes);

    for(int i=0; i<nTimes; i++)
    {
        stringstream ss;
        ss << setfill('0') << setw(6) << i;
        vstrImageLeft[i] = strPrefixLeft + ss.str() + ".png";
        vstrImageRight[i] = strPrefixRight + ss.str() + ".png";
        
        // 预检查文件是否存在
        if(!std::ifstream(vstrImageLeft[i]).good() || !std::ifstream(vstrImageRight[i]).good())
        {
            cerr << "WARNING: Image file not found: " 
                 << (!std::ifstream(vstrImageLeft[i]).good() ? vstrImageLeft[i] : vstrImageRight[i]) << endl;
        }
    }
}