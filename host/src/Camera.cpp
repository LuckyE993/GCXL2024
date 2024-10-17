//
// Created by luckye on 24-10-16.
//
#include <opencv2/opencv.hpp>
#include <thread>
#include <iostream>
#include <chrono>
#include <Camera.h>
using namespace cv;
using namespace std;
int frame_count = 0;

// 摄像头读取函数
void captureVideo(int cameraID)
{
    // 打开摄像头
    VideoCapture cap(cameraID); // 使用DirectShow来确保兼容USB免驱摄像头
    if (!cap.isOpened())
    {
        cerr << "无法打开摄像头!" << endl;
        return;
    }

    // 设置视频格式为MPEG
    cap.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'P', 'E', 'G'));
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    // 设置帧率为30FPS
    cap.set(CAP_PROP_FPS, 30);


    // 循环读取视频帧
    while (true)
    {
        Mat frame;
        cap >> frame; // 获取一帧

        if (frame.empty())
        {
            cerr << "读取帧失败!" << endl;
            break;
        }

        // imshow("Camera Video", frame);

        // 按下ESC键退出
        if (waitKey(10) == 27)
        {
            break;
        }
    }

    // 释放资源
    cap.release();
    destroyAllWindows();
}

// 定义全局变量
cv::Mat globalFrame;

// 构造函数，初始化摄像头
Camera::Camera(int cameraID) : isRunning(false)
{
    cap.open(cameraID); // 打开摄像头
    if (!cap.isOpened())
    {
        std::cerr << "无法打开摄像头!" << std::endl;
        exit(1);
    }

    // 设置视频格式为MPEG
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'P', 'E', 'G'));
    // 设置视频帧大小
    cap.set(CAP_PROP_FPS, 30);
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    // 设置帧率为30FPS
}

// 析构函数，确保线程停止并释放资源
Camera::~Camera()
{
    stopCapture();
    if (cap.isOpened())
    {
        cap.release();
    }
}

// 开始视频捕获
void Camera::startCapture()
{
    if (!isRunning)
    {
        isRunning = true;
        captureThread = std::thread(&Camera::captureVideo, this);
    }
}

// 停止视频捕获
void Camera::stopCapture()
{
    if (isRunning)
    {
        isRunning = false;
        if (captureThread.joinable())
        {
            captureThread.join();
        }
    }
}

// 捕获视频帧的函数
void Camera::captureVideo()
{
    while (isRunning)
    {
        cv::Mat frame;
        cap >> frame;

        if (!frame.empty())
        {
            globalFrame = frame.clone(); // 将最新帧赋值给全局变量

        }
    }
    cv::destroyAllWindows();
}

// 获取最新的帧
cv::Mat Camera::getFrame()
{
    return globalFrame; // 返回当前的全局帧
}
