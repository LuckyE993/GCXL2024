//
// Created by luckye on 24-10-18.
//
#include "Detector.h"

namespace YAML
{
    template<>
    struct convert<Scalar>
    {
        static Node encode(const Scalar &rhs)
        {
            Node node;
            for (int i = 0; i < 4; ++i)
            {
                node.push_back(rhs[i]);
            }
            return node;
        }

        static bool decode(const Node &node, Scalar &rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
            {
                return false;
            }
            rhs = Scalar(node[0].as<double>(), node[1].as<double>(), node[2].as<double>(), node[3].as<double>());
            return true;
        }
    };
}

Point calculateCenter(int x, int y, int w, int h)
{
    int center_x = x + w / 2;
    int center_y = y + h / 2;
    return {center_x, center_y};
}

vector<Point> findLargestContour(const vector<vector<Point> > &contours, double &max_perimeter)
{
    vector<Point> largest_contour;
    for (const auto &cnt: contours)
    {
        double perimeter = arcLength(cnt, true);
        if (perimeter > max_perimeter)
        {
            max_perimeter = perimeter;
            largest_contour = cnt;
        }
    }
    return largest_contour;
}

cv::Scalar getColorThreshold(const YAML::Node &thresholds, const std::string &color)
{
    return cv::Scalar(
        thresholds[color][0].as<int>(),
        thresholds[color][1].as<int>(),
        thresholds[color][2].as<int>()
    );
}


Detector::Detector()
{
}

Detector::~Detector()
{
}


void Detector::load_config(const std::string &config_file)
{
}

void Detector::Material_detect(const Mat &img, int color, const YAML::Node &config)
{
    YAML::Node Material_Thresholds = config["Materail_Thresholds"];

    int min_contour = config["min_contour"].as<int>();
    cv::Scalar lbc = Material_Thresholds["lower_blue_contour"].as<cv::Scalar>();
    cv::Scalar ubc = Material_Thresholds["upper_blue_contour"].as<cv::Scalar>();
    cv::Scalar lgc = Material_Thresholds["lower_green_contour"].as<cv::Scalar>();
    cv::Scalar ugc = Material_Thresholds["upper_green_contour"].as<cv::Scalar>();
    cv::Scalar lrc = Material_Thresholds["lower_red_contour"].as<cv::Scalar>();
    cv::Scalar urc = Material_Thresholds["upper_red_contour"].as<cv::Scalar>();

    cv::Mat hsv;
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;

    if (color == BLUE)
    {
        inRange(hsv, Scalar(lbc[0], lbc[1], lbc[2]), Scalar(ubc[0], ubc[1], ubc[2]), mask);
    } else if (color == GREEN)
    {
        inRange(hsv, Scalar(lgc[0], lgc[1], lgc[2]), Scalar(ugc[0], ugc[1], ugc[2]), mask);
    } else if (color == RED)
    {
        inRange(hsv, Scalar(lrc[0], lrc[1], lrc[2]), Scalar(urc[0], urc[1], urc[2]), mask);
    }

    if (Vision_Mode)
    {
        imshow("mask", mask);
    }

    vector<vector<Point> > contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    double max_perimeter = 0;
    vector<Point> largest_contour;

    // 遍历每个轮廓
    for (const auto &cnt: contours)
    {
        double perimeter = arcLength(cnt, true);
        if (perimeter > max_perimeter)
        {
            max_perimeter = perimeter;
            largest_contour = cnt;
        }
    }

    int x = 0, y = 0, w = 0, h = 0;

    if (!largest_contour.empty())
    {
        Rect boundingRect = cv::boundingRect(largest_contour);
        x = boundingRect.x;
        y = boundingRect.y;
        w = boundingRect.width;
        h = boundingRect.height;
        cv::Point center = calculateCenter(x, y, w, h);


        if (max_perimeter > min_contour && (x + w / 2) > 310)
        {
            object_data.position_matrix[0] = {x, y};
            object_data.position_matrix[1] = {x, y + h};
            object_data.position_matrix[2] = {x + w, y + h};
            object_data.position_matrix[3] = {x + 2, y};


            cv::Point center = calculateCenter(x, y, w, h);
        } else
        {
            x = 0;
            y = 0;
            w = 0;
            h = 0;
            object_data.position_matrix[0] = {x, y};
            object_data.position_matrix[1] = {x, y + h};
            object_data.position_matrix[2] = {x + w, y + h};
            object_data.position_matrix[3] = {x + 2, y};

            cv::Point center = calculateCenter(x, y, w, h);
        }
    } else
    {
        object_data.position_matrix[0] = {x, y};
        object_data.position_matrix[1] = {x, y + h};
        object_data.position_matrix[2] = {x + w, y + h};
        object_data.position_matrix[3] = {x + 2, y};

        int center_x = x + w / 2;
        int center_y = y + h / 2;

        cv::Point center = calculateCenter(x, y, w, h);
    }

    if (Vision_Mode)
    {
        cv::Point center = calculateCenter(x, y, w, h);
        rectangle(img, Point(x, y), Point(x + w, y + h), Scalar(0, 255, 0), 2);
        circle(img, center, 5, Scalar(0, 0, 0), -1);
        imshow("materail_img", img);
    }
}

void Detector::Land_mark_Detect(Mat img, int color, const YAML::Node &config)
{
}

void Detector::Material_detect_v2(const Mat &img, const YAML::Node &config)
{
    YAML::Node Material_Thresholds = config["Material_Thresholds"];
    auto min_perimeter_threshold = config["min_perimeter_threshold"].as<double>();
    // 正确地将 YAML 文件中的数组解析为 cv::Scalar
    cv::Scalar lbc = getColorThreshold(Material_Thresholds, "lower_blue_contour");
    cv::Scalar ubc = getColorThreshold(Material_Thresholds, "upper_blue_contour");
    cv::Scalar lgc = getColorThreshold(Material_Thresholds, "lower_green_contour");
    cv::Scalar ugc = getColorThreshold(Material_Thresholds, "upper_green_contour");
    cv::Scalar lrc = getColorThreshold(Material_Thresholds, "lower_red_contour");
    cv::Scalar urc = getColorThreshold(Material_Thresholds, "upper_red_contour");

    // 转换颜色空间
    cv::Mat hsv;
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

    // 生成颜色掩码
    cv::Mat mask_blue, mask_green, mask_red;
    inRange(hsv, lbc, ubc, mask_blue); // 蓝色掩码
    inRange(hsv, lgc, ugc, mask_green); // 绿色掩码
    inRange(hsv, lrc, urc, mask_red); // 红色掩码

    vector<vector<Point> > contours_blue, contours_green, contours_red;
    findContours(mask_blue, contours_blue, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    findContours(mask_green, contours_green, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    findContours(mask_red, contours_red, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    if (config["debug_flag"])
    {
        imshow("mask_blue", mask_blue);
        imshow("mask_green", mask_green);
        imshow("mask_red", mask_red);
    }

    vector<Point> largest_contour;
    double max_perimeter = 0.0; // 当前最大的周长

    // 查找蓝色轮廓中的最大轮廓
    double max_perimeter_blue = 0;
    vector<Point> largest_contour_blue = findLargestContour(contours_blue, max_perimeter_blue);
    if (max_perimeter_blue > max_perimeter && max_perimeter_blue > min_perimeter_threshold)
    {
        largest_contour = largest_contour_blue;
        max_perimeter = max_perimeter_blue;
        object_data.color = 0x05; // 蓝色
    }

    // 查找绿色轮廓中的最大轮廓
    double max_perimeter_green = 0;
    vector<Point> largest_contour_green = findLargestContour(contours_green, max_perimeter_green);
    if (max_perimeter_green > max_perimeter && max_perimeter_green > min_perimeter_threshold)
    {
        largest_contour = largest_contour_green;
        max_perimeter = max_perimeter_green;
        object_data.color = 0x04; // 绿色
    }

    // 查找红色轮廓中的最大轮廓
    double max_perimeter_red = 0;
    vector<Point> largest_contour_red = findLargestContour(contours_red, max_perimeter_red);
    if (max_perimeter_red > max_perimeter && max_perimeter_red > min_perimeter_threshold)
    {
        largest_contour = largest_contour_red;
        max_perimeter = max_perimeter_red;
        object_data.color = 0x03; // 红色
    }

    // 如果没有符合条件的轮廓
    if (max_perimeter <= min_perimeter_threshold)
    {
        largest_contour.clear(); // 清空轮廓
        object_data.color = 0x00; // 无效颜色
    }

    int x = 0, y = 0, w = 0, h = 0;
    if (!largest_contour.empty())
    {
        // 使用 boundingRect 从最大轮廓中获取边界框
        Rect bounding_box = boundingRect(largest_contour);

        // 提取边界框的 x, y, w, h
        x = bounding_box.x;
        y = bounding_box.y;
        w = bounding_box.width;
        h = bounding_box.height;
    }

    object_data.position_matrix[0] = {x, y};
    object_data.position_matrix[1] = {x, y + h};
    object_data.position_matrix[2] = {x + w, y + h};
    object_data.position_matrix[3] = {x + w, y};

    int center_x = x + w / 2;
    int center_y = y + h / 2;

    object_data.center = Point(center_x, center_y);

    // cout << "Detector: Centor: " << center_x << " " << center_y << "Color: " << object_data.color << endl;
    if (Vision_Mode)
    {
        circle(img, object_data.center, 5, Scalar(0, 0, 0), -1);
        //rectangle(img, Point(x, y), Point(x + w, y + h), Scalar(255, 239, 213), 1);
        imshow("materail_img", img);
    }
}


void Detector::Edge_Detect(Mat &img)
{
}

void Detector::Set_Object_Data()
{
}
