//
// Created by luckye on 24-10-18.
//

#ifndef DETECTOR_H
#define DETECTOR_H
#include <opencv2/opencv.hpp>
#include "iostream"
#include "Camera.h"
#include <string>
#include <cstring>
#include <yaml-cpp/yaml.h>
#define Vision_Mode true
using namespace std;
using namespace cv;

/*
 *
detect_camera.startCapture();
while (true)
{
    Mat frame = detect_camera.getFrame(); // 获取当前帧
    if (!frame.empty())
    {
        detector.Material_detect_v2(frame, config);
        // imshow("Frame", frame);
    }

    char key = waitKey(1);
    if (key == 'q')
    {
        break;
    }
    cout << "Detector: Centor: " << detector.object_data.center.x << " " << detector.object_data.center.y
            << "  Color: " << detector.object_data.color << endl;
}
 *
 */

// 目标物体参数
struct Object_Data
{
    std::vector<std::vector<int> > position_matrix;
    cv::Point center;
    int color;

    // 默认构造函数
    Object_Data() : position_matrix(4, std::vector<int>(2, 0)), center(0, 0), color(0)
    {
    }
};

struct Circle_Data
{
    cv::Point center;
    int color;
    // 默认构造函数
    Circle_Data() : center(0, 0)
    {
    }
};

class Detector
{
public:
    Detector();

    ~Detector();


    void load_config(const std::string &config_file);

    void Material_detect(const Mat &img, int color, const YAML::Node &config);

    void Land_mark_Detect(Mat img, int color, const YAML::Node &config);

    void Material_detect_v2(const Mat &img, const YAML::Node &config);

    void Land_mark_Detect_v2(const Mat &img, const YAML::Node &config);

    Point2d calculateAverage(const std::deque<Point> &points);

    bool checkIfStationary(const std::deque<int> &x_positions, int threshold, int size);

    int getMovementStatus(const Point &center, pair<int, int> range);

    // 成员变量
    Object_Data object_data;
    Circle_Data circle_data;

    static int RED;
    static int GREEN;
    static int BLUE;
};

#endif //DETECTOR_H
