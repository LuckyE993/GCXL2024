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

int sendLatecy = config["send_latecy_ms"].as<int>();

// 函数判断是否连续10次静止
bool checkIfStationary(const std::deque<int> &x_positions, int threshold, int size)
{
    if (x_positions.size() < size)
    {
        return false; // 如果记录的点数不足10个，返回false
    }

    int min_x = *min_element(x_positions.begin(), x_positions.end());
    int max_x = *max_element(x_positions.begin(), x_positions.end());

    // 如果最大值和最小值的差值小于阈值，认为物体静止
    return (max_x - min_x) <= threshold;
}

// 函数判断物体运动状态
int getMovementStatus(const Point &center, pair<int, int> range)
{
    if (center.x > range.first && center.x < range.second)
    {
        return 2;
    } else if (center.x < range.first)
    {
        return 3;
    } else if (center.x > range.second)
    {
        return 1;
    }
    return 0;
}

// 发送帧的函数
void sendFramePeriodically(WzSerialportPlus &serialport, int interval_ms)
{
    std::unique_lock<std::mutex> lock(mtx);
    while (serialThreadRunning)
    {
        condition.wait_for(lock, std::chrono::milliseconds(interval_ms));
        // 发送发送帧
        serialport.send((char *) &sendFrame, sizeof(sendFrame));
        serialport.printFrameInHex(sendFrame);
    }

    std::cout << "Serial thread stopping...\n";
}

void processMode(int mode, WzSerialportPlus &serialport,
                 QRcode &qrCodeScanner, Command &command,
                 Camera &qr_camera, Detector &detector,
                 Frame &sendFrame, YAML::Node &config,
                 int sendLatecy)
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
                // destroyWindow("QR Camera Video");
                // break;
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
            detect_camera.startCapture();
            Point previousCenter(640, 0);
            int move_status = 1; // 存储当前运动状态
            int move_range = 0;
            std::deque<int> x_positions; // 存储最近10个x坐标
            int stationary_threshold = 10; // 判断静止的阈值

            command.generateMaterialFrame(sendFrame, config);
            // 启动新的发送线程
            {
                std::lock_guard<std::mutex> lock(mtx);
                serialThreadRunning = true;
                serial_thread = std::thread(sendFramePeriodically, std::ref(serialport), sendLatecy);
            }
            while (!stop_previous_thread)
            {
                Mat frame = detect_camera.getFrame(); // 获取当前帧
                if (!frame.empty())
                {
                    detector.Material_detect_v2(frame, config);
                    x_positions.push_back(detector.object_data.center.x);

                    if (x_positions.size() > 50)
                    {
                        x_positions.pop_front(); // 保持队列大小为10
                    }

                    if (checkIfStationary(x_positions, stationary_threshold, 50)) // 静止
                    {
                        // cout << "Object is stationary based on the last 10 positions" << endl;
                        move_status = 2;
                    } else
                    {
                        move_status = 1;
                    }

                    // 判断当前中心点的位置范围
                    move_range = getMovementStatus(detector.object_data.center, make_pair(300, 340));
                    command.generateMaterialFrame(sendFrame,
                                                  config, detector.object_data.center.x, detector.object_data.center.y,
                                                  move_status, move_range, detector.object_data.color);
                }
            }

            detect_camera.stopCapture();
            break;
        }
        case 3:
        {
            clog << "[Serial] Received frame mode: 3" << endl;
            // TODO 地标圆心检测的实现
            detect_camera.startCapture();
            command.generateDetectFrame(sendFrame, config);
            // 启动新的发送线程
            {
                std::lock_guard<std::mutex> lock(mtx);
                serialThreadRunning = true;
                serial_thread = std::thread(sendFramePeriodically, std::ref(serialport), sendLatecy);
            }
            deque<Point> positions;
            while (!stop_previous_thread)
            {
                Mat frame = detect_camera.getFrame();
                if (!frame.empty())
                {
                    detector.Land_mark_Detect_v2(frame, config);
                    if (detector.circle_data.center.x == 0 || detector.circle_data.center.y == 0)
                    {
                        positions.clear();
                    } else
                    {
                        positions.push_back(detector.circle_data.center);
                    }
                    if (positions.size() > 10)
                    {
                        positions.pop_front();
                    }
                }
                Point2d average_position = detector.calculateAverage(positions);
                command.generateDetectFrame(sendFrame, config,
                                            average_position.x, average_position.y,
                                            detector.circle_data.color);
            }
            detect_camera.stopCapture();
            break;
        }
        default:
            cout << "[Serial] Unknown mode received: " << mode << endl;
            break;
    }
}


// 串口接收回调函数
void serialCallback(char *data, int length, WzSerialportPlus &serialport, QRcode &qrCodeScanner, Command &command,
                    Camera &qr_camera, Detector &detector, Frame &sendFrame, YAML::Node &config,
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
            // destroyWindow("QR Camera Video");

            this_thread::sleep_for(std::chrono::milliseconds(50));
            mode_thread = std::thread(processMode, receivedFrame->mode, std::ref(serialport),
                                      std::ref(qrCodeScanner), std::ref(command),
                                      std::ref(qr_camera), std::ref(detector), std::ref(sendFrame),
                                      std::ref(config), sendLatecy);

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
        cerr << "Received frame matches failed. Length error, length is " << sizeof(data) << endl;
    }
}

int main()
{
    serialport.setReceiveCalback([&](char *data, int length)
    {
        serialCallback(data, length, serialport, qrCodeScanner, command, qr_camera,
                       detector, sendFrame, config, sendLatecy);
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
