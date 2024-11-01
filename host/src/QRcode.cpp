//
// Created by luckye on 24-10-14.
//

#include <QRcode.h>
#include <csignal>

#include "Command.h"
using namespace std;
using namespace cv;

// 构造函数，初始化ZBar扫描器
QRcode::QRcode()
{
    scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);
}

// 处理传入的Mat图像，解析二维码
bool QRcode::processQRCode(const cv::Mat &frame)
{
    cv::Mat gray;
    // 将输入图像转换为灰度图
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // 将OpenCV的Mat转换为ZBar可处理的图像
    int width = gray.cols;
    int height = gray.rows;
    uchar *raw = (uchar *) gray.data;
    zbar::Image image(width, height, "Y800", raw, width * height);

    // 使用ZBar扫描器扫描二维码
    int n = scanner.scan(image);
    if (n == 0)
    {
        std::cerr << "未检测到二维码" << std::endl;
        return false;
    }

    // 处理扫描结果，假设只有一个二维码
    for (auto symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
    {
        std::string qrData = symbol->get_data();
        std::cout << "扫描到的二维码数据: " << qrData << std::endl;
        return parseQRCodeData(qrData);
    }

    return false;
}

// 解析二维码数据并存储为6个字节
bool QRcode::parseQRCodeData(const std::string &qrData)
{
    // 检查是否符合"123+321"格式
    if (qrData.size() != 7 || qrData[3] != '+')
    {
        std::cerr << "二维码格式不正确，要求格式为'123+321'" << std::endl;
        return false;
    }

    // 提取数字并转换为字节存储
    std::string firstPart = qrData.substr(0, 3); // 123
    std::string secondPart = qrData.substr(4, 3); // 321

    try
    {
        int firstNumber = std::stoi(firstPart);
        int secondNumber = std::stoi(secondPart);

        // 将数字拆分为字节存储
        qrDataBytes[0] = (firstNumber / 100) % 10;
        qrDataBytes[1] = (firstNumber / 10) % 10;
        qrDataBytes[2] = firstNumber % 10;

        qrDataBytes[3] = (secondNumber / 100) % 10;
        qrDataBytes[4] = (secondNumber / 10) % 10;
        qrDataBytes[5] = secondNumber % 10;
    } catch (std::exception &e)
    {
        std::cerr << "二维码数据解析失败: " << e.what() << std::endl;
        return false;
    }

    return true;
}

// 返回存储的6个字节数据
const std::array<uint8_t, 6> &QRcode::getBytes() const
{
    return qrDataBytes;
}

std::string QRcode::getCurrentTimeString()
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);

    // 格式化时间为字符串
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

void QRcode::startOrRestartShowQRInfo(const std::string &filename)
{
    // 检查 show_QR_info 是否已经启动
    if (showQRInfoPID > 0)
    {
        // 进程已经启动，尝试关闭
        if (kill(showQRInfoPID, SIGTERM) == 0)
        {
            std::cout << "Terminated existing show_QR_info process with PID: " << showQRInfoPID << std::endl;
        } else
        {
            std::cerr << "Failed to terminate show_QR_info process with PID: " << showQRInfoPID << std::endl;
        }
        showQRInfoPID = -1; // 清空记录的PID
    }

    // 使用 fork 和 execl 启动新的进程运行 show_QR_info 程序
    pid_t pid = fork();
    if (pid == -1)
    {
        std::cerr << "Fork failed!" << std::endl;
    } else if (pid == 0)
    {
        // 子进程
        execl("./show_QR_info", "show_QR_info", filename.c_str(), nullptr);
        std::cerr << "Failed to execute show_QR_info" << std::endl;
        exit(1); // 如果 exec 失败，退出子进程
    } else
    {
        // 父进程
        // 记录新启动的 show_QR_info 进程的 PID
        showQRInfoPID = pid;
        std::cout << "Started show_QR_info process with PID: " << showQRInfoPID << std::endl;
    }
}

void QRcode::generateQRImage(cv::Mat &qrimage, const std::array<uint8_t, 6> &bytes, const cv::Scalar &font_color,
                             int font, double font_scale, int font_thickness)
{
    // 构造文本
    std::string firstPart = std::to_string(bytes[0]) + std::to_string(bytes[1]) + std::to_string(bytes[2]);
    std::string secondPart = std::to_string(bytes[3]) + std::to_string(bytes[4]) + std::to_string(bytes[5]);
    std::string text = firstPart + "+" + secondPart;

    // 设置文本位置
    Point text_position(50, 50);
    putText(qrimage, text, text_position, font, font_scale, font_color, font_thickness, LINE_AA);


}

std::string QRcode::saveQRImage(const cv::Mat &image)
{
    std::string currentTime = getCurrentTimeString();
    std::string filename = currentTime + ".jpg";
    if (imwrite(filename, image))
    {
        std::cout << "Image saved successfully: " << filename << std::endl;
    } else
    {
        std::cerr << "Failed to save image: " << filename << std::endl;
    }
    return filename;
}
