//
// Created by luckye on 24-10-24.
//
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    // 打开摄像头0和摄像头2
    VideoCapture cap0(0);
    VideoCapture cap2(2);

    if (!cap0.isOpened()) {
        cout << "Error: Could not open camera 0" << endl;
        return -1;
    }

    if (!cap2.isOpened()) {
        cout << "Error: Could not open camera 2" << endl;
        return -1;
    }

    // 创建窗口
    namedWindow("Camera 0", WINDOW_AUTOSIZE);
    namedWindow("Camera 2", WINDOW_AUTOSIZE);

    while (true) {
        Mat frame0, frame2;

        // 读取两个摄像头的画面
        cap0 >> frame0;
        cap2 >> frame2;

        if (frame0.empty() || frame2.empty()) {
            cout << "Error: Could not grab frames from cameras" << endl;
            break;
        }

        // 显示两个摄像头的画面
        imshow("Camera 0", frame0);
        imshow("Camera 2", frame2);

        // 按下ESC键退出
        char key = (char)waitKey(30);
        if (key == 27) break;
    }

    // 释放摄像头
    cap0.release();
    cap2.release();
    destroyAllWindows();

    return 0;
}
