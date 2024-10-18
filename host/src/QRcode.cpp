//
// Created by luckye on 24-10-14.
//
#include <QRcode.h>
#include <iostream>
extern Camera camera;
// 构造函数，初始化ZBar扫描器
QRcode::QRcode() {
    scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);
}

// 处理传入的Mat图像，解析二维码
bool QRcode::processQRCode(const cv::Mat& frame) {
    cv::Mat gray;
    // 将输入图像转换为灰度图
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // 将OpenCV的Mat转换为ZBar可处理的图像
    int width = gray.cols;
    int height = gray.rows;
    uchar *raw = (uchar *)gray.data;
    zbar::Image image(width, height, "Y800", raw, width * height);

    // 使用ZBar扫描器扫描二维码
    int n = scanner.scan(image);
    if (n == 0) {
        std::cerr << "未检测到二维码" << std::endl;
        return false;
    }

    // 处理扫描结果，假设只有一个二维码
    for (auto symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
        std::string qrData = symbol->get_data();
        std::cout << "扫描到的二维码数据: " << qrData << std::endl;
        return parseQRCodeData(qrData);
    }

    return false;
}

// 解析二维码数据并存储为6个字节
bool QRcode::parseQRCodeData(const std::string& qrData) {
    // 检查是否符合"123-321"格式
    if (qrData.size() != 7 || qrData[3] != '-') {
        std::cerr << "二维码格式不正确，要求格式为'123-321'" << std::endl;
        return false;
    }

    // 提取数字并转换为字节存储
    std::string firstPart = qrData.substr(0, 3);  // 123
    std::string secondPart = qrData.substr(4, 3); // 321

    try {
        int firstNumber = std::stoi(firstPart);
        int secondNumber = std::stoi(secondPart);

        // 将数字拆分为字节存储
        qrDataBytes[0] = (firstNumber / 100) % 10;
        qrDataBytes[1] = (firstNumber / 10) % 10;
        qrDataBytes[2] = firstNumber % 10;

        qrDataBytes[3] = (secondNumber / 100) % 10;
        qrDataBytes[4] = (secondNumber / 10) % 10;
        qrDataBytes[5] = secondNumber % 10;
    } catch (std::exception& e) {
        std::cerr << "二维码数据解析失败: " << e.what() << std::endl;
        return false;
    }

    return true;
}

// 返回存储的6个字节数据
const std::array<uint8_t, 6>& QRcode::getBytes() const {
    return qrDataBytes;
}


