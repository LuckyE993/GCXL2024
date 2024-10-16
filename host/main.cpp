#include "usart.hpp"
#include "iostream"
#include "yaml-cpp/yaml.h"
#include "string"
#include <cstring>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <Camera.h>

using namespace cv;
using namespace std;
int num;
int receive_num;

const string config_dir = "../config.yaml";

void sendFramePeriodically(WzSerialportPlus &serialport, const Frame &sendFrame, int interval_ms)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));

        // 发送发送帧
        serialport.send((char *) &sendFrame, sizeof(sendFrame));
        std::cout << num++ << "Frame sent at interval of " << interval_ms << " ms." << std::endl;
    }
}

int main()
{
    YAML::Node config = YAML::LoadFile(config_dir);

    WzSerialportPlus serialport;
    Frame sendFrame = serialport.initSendFrame(config);
    Frame receiveFrame = serialport.initReceiveFrame(config);

    serialport.setReceiveCalback([&](char *data, int length)
    {
        printf("received: %s\n", data);

        if (length == sizeof(Frame))
        {
            Frame *receivedFrame = (Frame *) data;
            if (receivedFrame->head == receiveFrame.head && receivedFrame->tail == receiveFrame.tail)
            {
                receive_num++;
                std::clog << receive_num << "  Received frame matches success;" << std::endl;
                //TODO 解析数据


                for (int i = 1; i < sizeof(Frame); i++)
                {
                    std::cout << "Received data: " << static_cast<int>(data[i]) << std::endl;
                }
            } else
            {
                std::cerr << "Received frame matches failed. Frame head or tail error" << std::endl;
            }
        } else
        {
            std::cerr << "Received frame matches failed. Length err len: " << length << "Frame len: " << sizeof(Frame)
                    << std::endl;
        }
        // serialport.send((char *) &sendFrame, sizeof(sendFrame));
    });

    serialport.initSerialFromConfig(serialport, config_dir);

    // std::thread sendThread(sendFramePeriodically, std::ref(serialport), std::ref(sendFrame), 500);

    Camera camera(0); // 初始化摄像头
    camera.startCapture(); // 开始捕获视频

    cout << "Init Success! " << endl;

    while (true)
    {
        getchar();
        serialport.close();
        // sendThread.join();
        break;
    }

    return 0;
}
