//
// Created by luckye on 24-10-16.
//

#ifndef CAMERA_H
#define CAMERA_H
#include <thread>
/*
*
while (true)
{
    Mat frame = camera.getFrame(); // 获取当前帧
    if (!frame.empty())
    {
        // 在这里对帧进行处理或显示
        imshow("Frame", frame);
    }

    char key = waitKey(1);
    if (key == 'q')
    {
        break;
    }
}
 *
 *
 */

void captureVideo(int cameraID);
class Camera {
public:
    // 构造函数和析构函数
    Camera(int cameraID);
    ~Camera();

    // 开始获取视频
    void startCapture();

    // 停止获取视频
    void stopCapture();

    // 获取最新的帧
    cv::Mat getFrame();

    void captureVideo(); // 负责视频获取的线程函数
    cv::VideoCapture cap;
    std::thread captureThread;
    bool isRunning;
};

// 全局的Mat变量，用于存储摄像头帧
extern cv::Mat globalFrame;
#endif //CAMERA_H
