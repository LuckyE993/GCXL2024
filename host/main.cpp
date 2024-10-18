#include "usart.hpp"
#include "iostream"
#include "yaml-cpp/yaml.h"
#include "string"
#include <cstring>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <Camera.h>
#include <QRcode.h>
#include "Command.h"
#include <condition_variable>

using namespace cv;
using namespace std;


const string config_dir = "../config.yaml";

YAML::Node config = YAML::LoadFile(config_dir);
Camera camera(config["cam_QRCode"].as<int>()); // 初始化摄像头
WzSerialportPlus serialport;
QRcode qrCodeScanner;
Command command;
thread serial_thread;
volatile bool serialThreadRunning = true;
std::mutex mtx;
std::condition_variable condition;

Frame sendFrame = serialport.initSendFrame(config);
Frame receiveFrame = serialport.initReceiveFrame(config);

int debugFlag = config["debug_flag"].as<int>();
int sendLatecy = config["send_latecy_ms"].as<int>();


// 发送帧的函数
void sendFramePeriodically(WzSerialportPlus &serialport, int interval_ms) {
    std::unique_lock<std::mutex> lock(mtx);
    while (serialThreadRunning) {
        condition.wait_for(lock, std::chrono::milliseconds(interval_ms));

        if (!serialThreadRunning) break;

        // 发送发送帧
        serialport.send((char *) &sendFrame, sizeof(sendFrame));
        std::cout << "Frame sent at interval of " << interval_ms << " ms." << std::endl;
    }
    std::cout << "Serial thread stopping...\n";
}

int main()
{
    /*--------------------------------------------------  Camera  -------------------------------------------------*/



    /*--------------------------------------------------  Camera  -------------------------------------------------*/


    serialport.setReceiveCalback([&](char *data, int length)
    {
        printf("received: %s\n", data);

        if (length == sizeof(Frame))
        {
            Frame *receivedFrame = (Frame *) data;
            if (receivedFrame->head == receiveFrame.head && receivedFrame->tail == receiveFrame.tail)
            {
                clog << "  Received frame matches success;" << endl;
                //TODO 解析数据
                switch (receivedFrame->mode)
                {
                    case 0:
                    {
                        cout << "Received frame mode: 0 Stop send." << endl;
                        if (serial_thread.joinable()) {
                            std::cout << "Thread has started, attempting to stop it.\n";

                            // 停止线程
                            {
                                std::lock_guard<std::mutex> lock(mtx);
                                serialThreadRunning = false;  // 设置标志位为false
                            }
                            condition.notify_all();  // 唤醒阻塞的线程
                            serial_thread.join();  // 等待线程结束
                            std::cout << "Thread has been stopped.\n";
                        } else {
                            std::cout << "The thread has not started.\n";
                        }
                        break;
                    }
                    case 1:
                        //任务码如果没扫到仍然不能通过00进行暂停，原因是主进程卡在while循环。
                    {
                        cout << "Received frame mode: 1 QRCode" << endl;
                        camera.startCapture(); // 开始捕获视频
                        this_thread::sleep_for(std::chrono::milliseconds(150));
                        while (true)
                        {
                            Mat frame = camera.getFrame(); // 获取当前帧

                            // 检查是否成功获取帧
                            if (frame.empty())
                            {
                                cerr << "无法获取帧!" << endl;
                                continue;
                            }
                            if (qrCodeScanner.processQRCode(frame))
                            {
                                const array<uint8_t, 6> &bytes = qrCodeScanner.getBytes();
                                cout << "解析后的6个字节: ";
                                for (const auto &byte: bytes)
                                {
                                    cout << (int) byte << " ";
                                }

                                if (command.generateQRcodeFrame(sendFrame, config, bytes))
                                    cout << "generateQRcodeFrame success" << endl;
                                else
                                    cerr << "generateQRcodeFrame failed" << endl;
                                // 显示帧
                                if (debugFlag == 1)
                                    imshow("Camera Video", frame);
                                break;
                            }
                        }

                        // 启动线程前，确保旧线程已停止
                        if (serial_thread.joinable()) {
                            {
                                std::lock_guard<std::mutex> lock(mtx);
                                serialThreadRunning = false;
                            }
                            condition.notify_all();  // 唤醒阻塞的线程
                            serial_thread.join();  // 等待线程结束
                        }

                        // 重置状态并启动新线程
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            serialThreadRunning = true;
                        }
                        serial_thread = std::thread(sendFramePeriodically, ref(serialport), sendLatecy);
                        break;
                    }
                    case 2:
                        //TODO 物料检测
                        cout << "Received frame mode: 2" << endl;
                        break;
                    case 3:
                        //TODO 地标圆心检测
                        cout << "Received frame mode: 3" << endl;
                        break;
                }

                for (int i = 1; i < sizeof(Frame); i++)
                {
                    cout << "Received data: " << static_cast<int>(data[i]) << endl;
                }
            } else
            {
                cerr << "Received frame matches failed. Frame head or tail error" << endl;
            }
        } else
        {
            cerr << "Received frame matches failed. Length err len: " << length
                    << "Frame len: " << sizeof(Frame) << endl;
        }
    });

    serialport.initSerialFromConfig(serialport, config_dir);


    cout << "Init Success! " << endl;

    while (true)
    {
        getchar();
        serialport.close();
        break;
    }

    return 0;
}
