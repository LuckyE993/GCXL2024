#include <opencv2/opencv.hpp>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <filesystem>  // C++17标准库，用于创建文件夹
#include <string>

namespace fs = std::filesystem;

// 获取当前时间字符串，精确到毫秒
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y_%m_%d_%H_%M_%S");
    ss << "_" << std::setfill('0') << std::setw(3) << milliseconds.count();
    return ss.str();
}

int main(int argc, char** argv) {
    // 检查是否传入自定义保存目录参数
    std::string folder = "images";  // 默认保存路径
    if (argc > 1) {
        folder = argv[1];
    }

    // 创建保存目录（如果不存在）
    if (!fs::exists(folder)) {
        fs::create_directories(folder);
    }

    // 打开摄像头
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "无法打开摄像头" << std::endl;
        return -1;
    }

    // 设置分辨率为640x480
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::Mat frame;
    while (true) {
        // 读取帧
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "无法读取帧" << std::endl;
            break;
        }

        // 获取当前时间，并将其作为文件名
        std::string filename = folder + "/" + getCurrentTime() + ".jpg";

        // 保存图片
        cv::imwrite(filename, frame);
        std::cout << "保存图片: " << filename << std::endl;

        // 显示保存的图片
        cv::imshow("Saved Image", frame);

        // 每隔500ms保存一次
        if (cv::waitKey(500) == 27) {  // 按下ESC键退出
            break;
        }
    }

    return 0;
}
