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
#include <Detector.h>


using namespace cv;
using namespace std;


const string config_dir = "../config.yaml";

YAML::Node config = YAML::LoadFile(config_dir);
Camera qr_camera(config["cam_QRCode"].as<int>()); // 初始化摄像头
Camera detect_camera(config["cam_Det"].as<int>()); // 初始化摄像头
WzSerialportPlus serialport;
QRcode qrCodeScanner;
Command command;
Detector detector;
thread serial_thread;
thread mode_thread;
volatile bool serialThreadRunning = false;
bool stop_previous_thread = false; // 标志位，用来通知线程终止
mutex mtx;
condition_variable condition;

Frame sendFrame = serialport.initSendFrame(config);
Frame receiveFrame = serialport.initReceiveFrame(config);

int debugFlag = config["debug_flag"].as<int>();
int sendLatecy = config["send_latecy_ms"].as<int>();


// 发送帧的函数
void sendFramePeriodically(WzSerialportPlus &serialport, int interval_ms)
{
    std::unique_lock<std::mutex> lock(mtx);
    while (serialThreadRunning)
    {
        condition.wait_for(lock, std::chrono::milliseconds(interval_ms));


        // 发送发送帧
        serialport.send((char *) &sendFrame, sizeof(sendFrame));
        std::cout << "Frame sent at interval of " << interval_ms << " ms." << std::endl;
    }
    std::cout << "Serial thread stopping...\n";
}

void processMode(int mode, WzSerialportPlus &serialport,
                 QRcode &qrCodeScanner, Command &command,
                 Camera &qr_camera, Detector &detector,
                 Frame &sendFrame, YAML::Node &config,
                 int debugFlag, int sendLatecy)
{
    stop_previous_thread = false; // 新线程开始时，标志位复位

    switch (mode)
    {
        case 0:
        {
            cout << "[Serial] Received frame mode: 0 Stop send." << endl;
            if (serial_thread.joinable())
            {
                std::cout << "Thread has started, attempting to stop it.\n";

                // 停止发送线程
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    serialThreadRunning = false; // 设置标志位为false
                }
                condition.notify_all(); // 唤醒阻塞的线程
                serial_thread.join(); // 等待线程结束
                std::cout << "[Serial] Thread has been stopped.\n";
            } else
            {
                std::cout << "[Serial] The thread has not started.\n";
            }
            break;
        }
        case 1:
        {
            cout << "[Serial] Received frame mode: 1 QRCode" << endl;
            qr_camera.startCapture(); // 开始捕获视频
            std::this_thread::sleep_for(std::chrono::milliseconds(150));

            while (!stop_previous_thread)
            {
                Mat frame = qr_camera.getFrame(); // 获取当前帧

                if (frame.empty())
                {
                    cerr << "无法获取帧!" << endl;
                    continue;
                }
                imshow("QR Camera Video", frame);
                waitKey(1); // 等待键盘事件
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
                    break;
                }
            }

            qr_camera.stopCapture();
            if (stop_previous_thread)
            {
                destroyWindow("QR Camera Video");
                break;
            }
            // 启动新线程前，确保旧线程已停止
            if (serial_thread.joinable())
            {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    serialThreadRunning = false;
                }
                condition.notify_all(); // 唤醒阻塞的线程
                serial_thread.join(); // 等待线程结束
            }

            // 启动新的发送线程
            {
                std::lock_guard<std::mutex> lock(mtx);
                serialThreadRunning = true;
                serial_thread = std::thread(sendFramePeriodically, std::ref(serialport), sendLatecy);
            }
            break;
        }
        case 2:
        {
            clog << "[Serial] Received frame mode: 2" << endl;
           
            break;
        }
        case 3:
            clog << "[Serial] Received frame mode: 3" << endl;
        // TODO 地标圆心检测的实现
            break;
        default:
            cout << "[Serial] Unknown mode received: " << mode << endl;
    }
}


// 串口接收回调函数
void serialCallback(char *data, int length, WzSerialportPlus &serialport, QRcode &qrCodeScanner, Command &command,
                    Camera &qr_camera, Detector &detector, Frame &sendFrame, YAML::Node &config, int debugFlag,
                    int sendLatecy)
{
    if (length == sizeof(Frame))
    {
        Frame *receivedFrame = (Frame *) data;
        if (receivedFrame->head == sendFrame.head && receivedFrame->tail == sendFrame.tail)
        {
            clog << "Received frame matches success;" << endl;
            Frame confirm_frame = command.generateConfirmFrame(config);
            serialport.send((char *) &confirm_frame, sizeof(confirm_frame));

            if (serial_thread.joinable())
            {
                std::cout << "Thread has started, attempting to stop it.\n";

                // 停止发送线程
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    serialThreadRunning = false; // 设置标志位为false
                }
                condition.notify_all(); // 唤醒阻塞的线程
                serial_thread.join(); // 等待线程结束
                std::cout << "Thread has been stopped.\n";
            }
            // 将处理模式的逻辑移动到新的线程中
            if (mode_thread.joinable())
            {
                stop_previous_thread = true; // 通知前一个线程终止
                mode_thread.detach(); // 使得线程可以独立运行，不用等待它结束
            }
            destroyWindow("QR Camera Video");

            this_thread::sleep_for(std::chrono::milliseconds(50));
            mode_thread = std::thread(processMode, receivedFrame->mode, std::ref(serialport),
                                      std::ref(qrCodeScanner), std::ref(command),
                                      std::ref(qr_camera), std::ref(detector), std::ref(sendFrame),
                                      std::ref(config), debugFlag, sendLatecy);

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
        cerr << "Received frame matches failed. Length error." << endl;
    }
}

int main()
{
    /*--------------------------------------------------  Camera  -------------------------------------------------*/

    /*--------------------------------------------------  Camera  -------------------------------------------------*/


    serialport.setReceiveCalback([&](char *data, int length)
    {
        serialCallback(data, length, serialport, qrCodeScanner, command, qr_camera, detector, sendFrame, config,
                       debugFlag, sendLatecy);
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

/*printf("received: %s\n", data);

        if (length == sizeof(Frame))
        {
            Frame *receivedFrame = (Frame *) data;
            if (receivedFrame->head == receiveFrame.head && receivedFrame->tail == receiveFrame.tail)
            {
                clog << "  Received frame matches success;" << endl;
                Frame confirm_frame = command.generateConfirmFrame(config);
                serialport.send((char *) &confirm_frame, sizeof(confirm_frame));

                //TODO mode0 单独考虑
                if (receivedFrame->mode == 0x00)
                {
                    cout << "Received frame mode: 0 Stop send." << endl;
                    if (serial_thread.joinable())
                    {
                        std::cout << "Thread has started, attempting to stop it.\n";

                        // 停止线程
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            serialThreadRunning = false; // 设置标志位为false
                        }
                        condition.notify_all(); // 唤醒阻塞的线程
                        serial_thread.join(); // 等待线程结束
                        std::cout << "Thread has been stopped.\n";
                    } else
                    {
                        std::cout << "The thread has not started.\n";
                    }
                }
                //TODO mode123 放在一个 switch 线程里

                std::thread modeSwitch([&]()
                {
                    switch (receivedFrame->mode)
                    {
                        case 1:
                            //任务码如果没扫到仍然不能通过00进行暂停，原因是主进程卡在while循环。
                        {
                            cout << "Received frame mode: 1 QRCode" << endl;
                            qr_camera.startCapture(); // 开始捕获视频
                            this_thread::sleep_for(std::chrono::milliseconds(150));

                            while (true)
                            {
                                Mat frame = qr_camera.getFrame(); // 获取当前帧

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
                            qr_camera.stopCapture();
                            // 启动线程前，确保旧线程已停止
                            if (serial_thread.joinable())
                            {
                                {
                                    std::lock_guard<std::mutex> lock(mtx);
                                    serialThreadRunning = false;
                                }
                                condition.notify_all(); // 唤醒阻塞的线程
                                serial_thread.join(); // 等待线程结束
                            }

                            // 重置状态并启动新线程
                            {
                                std::lock_guard<std::mutex> lock(mtx);
                                serialThreadRunning = true;
                                serial_thread = std::thread(sendFramePeriodically, std::ref(serialport), sendLatecy);
                            }
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
                        default:
                            break;
                    }
                });
                modeSwitch.detach();


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
        }*/
