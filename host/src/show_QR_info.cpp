//
// Created by luckye on 24-10-29.
//
#include <opencv2/opencv.hpp>
#include <iostream>

void showQRCode(const std::string &filename)
{
    // 读取图像
    cv::Mat image = cv::imread(filename);

    // 检查图像是否成功加载
    if (image.empty())
    {
        std::cerr << "Could not open or find the image: " << filename << std::endl;
        return;
    }

    // 创建窗口并显示图像
    cv::namedWindow("QR Code", cv::WINDOW_AUTOSIZE);
    cv::imshow("QR Code", image);

    // 等待用户按键
    cv::waitKey(0); // 0表示无限等待，直到用户按键
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    showQRCode(filename);

    return 0;
}
