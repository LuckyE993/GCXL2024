//
// Created by luckye on 24-10-24.
//
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// 初始化阈值
int low_H = 0, low_S = 0, low_V = 0;
int high_H = 179, high_S = 255, high_V = 255;

// 回调函数（这里不需要执行其他操作）
void on_trackbar(int, void *)
{
}

int main()
{
    // 打开摄像头
    VideoCapture cap(2);
    // 设置视频格式为MPEG
    cap.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'P', 'E', 'G'));
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    // 设置帧率为30FPS
    cap.set(CAP_PROP_FPS, 30);

    if (!cap.isOpened())
    {
        cout << "Error: Could not open camera" << endl;
        return -1;
    }


    // 创建窗口
    namedWindow("HSV Adjustment", WINDOW_AUTOSIZE);

    // 创建Trackbars
    createTrackbar("Low H", "HSV Adjustment", &low_H, 179, on_trackbar);
    createTrackbar("Low S", "HSV Adjustment", &low_S, 255, on_trackbar);
    createTrackbar("Low V", "HSV Adjustment", &low_V, 255, on_trackbar);
    createTrackbar("High H", "HSV Adjustment", &high_H, 179, on_trackbar);
    createTrackbar("High S", "HSV Adjustment", &high_S, 255, on_trackbar);
    createTrackbar("High V", "HSV Adjustment", &high_V, 255, on_trackbar);

    while (true)
    {
        Mat frame, hsv_frame, mask;

        // 读取一帧
        cap >> frame;
        if (frame.empty())
        {
            cout << "Error: Could not grab a frame" << endl;
            break;
        }

        // 转换为HSV颜色空间
        cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

        // 应用阈值进行过滤
        inRange(hsv_frame, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), mask);

        // 显示图像
        imshow("Original", frame);
        imshow("Mask", mask);

        // 按下ESC键退出
        char key = (char) waitKey(30);
        if (key == 27) break;
    }

    // 释放摄像头
    cap.release();
    destroyAllWindows();

    return 0;
}
