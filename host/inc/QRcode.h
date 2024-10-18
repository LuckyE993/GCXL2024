//
// Created by luckye on 24-10-14.
//

#ifndef QRCODE_H
#define QRCODE_H
#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <string>
#include <Camera.h>

/*
*    // 循环读取视频帧
    while (true)
    {
        Mat frame = camera.getFrame(); // 获取当前帧

        // 检查是否成功获取帧
        if (frame.empty())
        {
            std::cerr << "无法获取帧!" << std::endl;
            continue;
        }
        if (qrCodeScanner.processQRCode(frame))
        {
            const std::array<uint8_t, 6> &bytes = qrCodeScanner.getBytes();
            std::cout << "解析后的6个字节: ";
            for (const auto &byte: bytes)
            {
                std::cout << (int) byte << " ";
            }
            std::cout << std::endl;
        }

        // 显示帧
        imshow("Camera Video", frame);

        // 按下ESC键退出
    }
 *
 */


class QRcode
{
public:
    QRcode(); // 构造函数
    bool processQRCode(const cv::Mat &frame); // 处理二维码图像并解析
    const std::array<uint8_t, 6> &getBytes() const; // 获取6个byte数据

private:
    zbar::ImageScanner scanner; // ZBar扫描器
    std::array<uint8_t, 6> qrDataBytes; // 存储二维码解析后的6个字节
    bool parseQRCodeData(const std::string &qrData); // 解析二维码数据
};


#endif //QRCODE_H
