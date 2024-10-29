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
}

void Detector::Land_mark_Detect(Mat img, int color, const YAML::Node &config)
{
    YAML::Node Landmark_Thresholds = config["Landmark_Thresholds"];
    circle_data.color = color;
    // 正确地将 YAML 文件中的数组解析为 cv::Scalar
    cv::Scalar lbc = getColorThreshold(Landmark_Thresholds, "lower_blue_circle");
    cv::Scalar ubc = getColorThreshold(Landmark_Thresholds, "upper_blue_circle");
    cv::Scalar lgc = getColorThreshold(Landmark_Thresholds, "lower_green_circle");
    cv::Scalar ugc = getColorThreshold(Landmark_Thresholds, "upper_green_circle");
    cv::Scalar lrc1 = getColorThreshold(Landmark_Thresholds, "lower_red_1_circle");
    cv::Scalar urc1 = getColorThreshold(Landmark_Thresholds, "upper_red_1_circle");
    cv::Scalar lrc2 = getColorThreshold(Landmark_Thresholds, "lower_red_2_circle");
    cv::Scalar urc2 = getColorThreshold(Landmark_Thresholds, "upper_red_2_circle");

    // 定义颜色常量
    const int BLUE = config["Color"]["blue"].as<int>();
    const int GREEN = config["Color"]["green"].as<int>();
    const int RED = config["Color"]["red"].as<int>();

    // 转换图像为 HSV 颜色空间
    Mat hsv;

    cvtColor(img, hsv, COLOR_BGR2HSV);

    cv::Mat mask, res;
    if (color == BLUE)
    {
        cv::inRange(hsv, lbc, ubc, mask);
        cv::bitwise_and(img, img, res, mask);
    } else if (color == GREEN)
    {
        cv::inRange(hsv, lgc, ugc, mask);
        cv::bitwise_and(img, img, res, mask);
    } else if (color == RED)
    {
        cv::Mat mask_red_1, mask_red_2;
        cv::inRange(hsv, lrc1, urc1, mask_red_1);
        cv::inRange(hsv, lrc2, urc2, mask_red_2);
        cv::bitwise_or(mask_red_1, mask_red_2, mask);
        cv::bitwise_and(img, img, res, mask);
    }

    // 将结果转换为灰度图像
    Mat img_gray;
    cvtColor(res, img_gray, COLOR_BGR2GRAY);
    medianBlur(img_gray, img_gray, 5);
    if (config["debug_flag"].as<int>())
    {
        imshow("img_gray", img_gray);
    }
    // 使用霍夫圆变换检测圆形
    vector<Vec3f> circles;
    HoughCircles(img_gray, circles, HOUGH_GRADIENT_ALT, 1, 20, 50, 0.8, 30, 0);

    Vec3f smallest_circle;

    int min_radius = config["min_radius"].as<int>();

    if (!circles.empty())
    {
        for (const auto &cnt: circles)
        {
            int radius = cnt[2];
            if (radius > min_radius)
            {
                min_radius = radius;
                smallest_circle = cnt;
            }
        }

        if (config["debug_flag"].as<int>())
        {
            circle(img, Point(smallest_circle[0], smallest_circle[1]), smallest_circle[2], Scalar(0, 255, 0), 2);
        }

        int x = smallest_circle[0];
        int y = smallest_circle[1];

        circle_data.center = Point(x, y);

        if (config["debug_flag"].as<int>())
        {
            circle(img, circle_data.center, 5, Scalar(0, 0, 0), -1);
            imshow("Landmark_img", img);
        }
    } else
    {
        circle_data.center = Point(0, 0);

        if (config["debug_flag"].as<int>())
        {
            imshow("Landmark_img", img);
        }
    }

    if (config["debug_flag"].as<int>())
    {
        waitKey(1);
    }
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
    cv::Scalar lrc = getColorThreshold(Material_Thresholds, "lower_red_1_contour");
    cv::Scalar urc = getColorThreshold(Material_Thresholds, "upper_red_1_contour");

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

    if (config["debug_flag"].as<int>())
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
        object_data.color = 0x03; // 蓝色
    }

    // 查找绿色轮廓中的最大轮廓
    double max_perimeter_green = 0;
    vector<Point> largest_contour_green = findLargestContour(contours_green, max_perimeter_green);
    if (max_perimeter_green > max_perimeter && max_perimeter_green > min_perimeter_threshold)
    {
        largest_contour = largest_contour_green;
        max_perimeter = max_perimeter_green;
        object_data.color = 0x02; // 绿色
    }

    // 查找红色轮廓中的最大轮廓
    double max_perimeter_red = 0;
    vector<Point> largest_contour_red = findLargestContour(contours_red, max_perimeter_red);
    if (max_perimeter_red > max_perimeter && max_perimeter_red > min_perimeter_threshold)
    {
        largest_contour = largest_contour_red;
        max_perimeter = max_perimeter_red;
        object_data.color = 0x01; // 红色
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
    if (config["debug_flag"].as<int>())
    {
        circle(img, object_data.center, 5, Scalar(0, 0, 0), -1);
        //rectangle(img, Point(x, y), Point(x + w, y + h), Scalar(255, 239, 213), 1);
        imshow("materail_img", img);
        waitKey(1);
    }
}

void Detector::Land_mark_Detect_v2(const Mat &img, const YAML::Node &config)
{
    YAML::Node Landmark_Thresholds = config["Landmark_Thresholds"];
    circle_data.color = 0x00;
    // 正确地将 YAML 文件中的数组解析为 cv::Scalar
    cv::Scalar lbc = getColorThreshold(Landmark_Thresholds, "lower_blue_circle");
    cv::Scalar ubc = getColorThreshold(Landmark_Thresholds, "upper_blue_circle");
    cv::Scalar lgc = getColorThreshold(Landmark_Thresholds, "lower_green_circle");
    cv::Scalar ugc = getColorThreshold(Landmark_Thresholds, "upper_green_circle");
    cv::Scalar lrc1 = getColorThreshold(Landmark_Thresholds, "lower_red_1_circle");
    cv::Scalar urc1 = getColorThreshold(Landmark_Thresholds, "upper_red_1_circle");
    cv::Scalar lrc2 = getColorThreshold(Landmark_Thresholds, "lower_red_2_circle");
    cv::Scalar urc2 = getColorThreshold(Landmark_Thresholds, "upper_red_2_circle");

    // 定义颜色常量
    const int BLUE = config["Color"]["blue"].as<int>();
    const int GREEN = config["Color"]["green"].as<int>();
    const int RED = config["Color"]["red"].as<int>();

    Mat res;
    // 转换图像为 HSV 颜色空间
    Mat hsv;
    cvtColor(img, hsv, COLOR_BGR2HSV);

    // 定义变量用于记录最大轮廓信息
    double max_area = 0.0;
    std::vector<Point> largest_contour;
    int detected_color = 0;

    // 检测蓝色
    Mat blue_mask, blue_res;
    inRange(hsv, lbc, ubc, blue_mask);
    std::vector<std::vector<Point> > blue_contours;
    findContours(blue_mask, blue_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (const auto &contour: blue_contours)
    {
        double area = contourArea(contour);
        if (area > max_area)
        {
            max_area = area;
            largest_contour = contour;
            detected_color = BLUE;
        }
    }

    // 检测绿色
    Mat green_mask;
    inRange(hsv, lgc, ugc, green_mask);
    std::vector<std::vector<Point> > green_contours;
    findContours(green_mask, green_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (const auto &contour: green_contours)
    {
        double area = contourArea(contour);
        if (area > max_area)
        {
            max_area = area;
            largest_contour = contour;
            detected_color = GREEN;
        }
    }

    // 检测红色
    Mat red_mask_1, red_mask_2, red_mask;
    inRange(hsv, lrc1, urc1, red_mask_1);
    inRange(hsv, lrc2, urc2, red_mask_2);
    bitwise_or(red_mask_1, red_mask_2, red_mask);
    std::vector<std::vector<Point> > red_contours;
    findContours(red_mask, red_contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (const auto &contour: red_contours)
    {
        double area = contourArea(contour);
        if (area > max_area)
        {
            max_area = area;
            largest_contour = contour;
            detected_color = RED;
        }
    }

    // 如果检测到任何轮廓，记录最大的轮廓信息
    if (!largest_contour.empty())
    {
        if (detected_color == BLUE)
        {
            bitwise_and(img, img, res, blue_mask);
        } else if (detected_color == GREEN)
        {
            bitwise_and(img, img, res, green_mask);
        } else if (detected_color == RED)
        {
            bitwise_and(img, img, res, red_mask);
        }
    } else
    {
        return;
    }

    // 将结果转换为灰度图像
    Mat img_gray;
    cvtColor(res, img_gray, COLOR_BGR2GRAY);
    medianBlur(img_gray, img_gray, 3);
    if (config["debug_flag"].as<int>())
    {
        imshow("img_gray", img_gray);
    }
    // 使用霍夫圆变换检测圆形
    vector<Vec3f> circles;
    HoughCircles(img_gray, circles, HOUGH_GRADIENT_ALT, 1, 20, 50, 0.8, 30, 0);

    Vec3f smallest_circle;

    int min_radius = config["min_radius"].as<int>();

    if (!circles.empty())
    {
        for (const auto &cnt: circles)
        {
            int radius = cnt[2];
            if (radius > min_radius)
            {
                min_radius = radius;
                smallest_circle = cnt;
            }
        }

        // if (config["debug_flag"].as<int>())
        // {
        //     circle(img, Point(smallest_circle[0], smallest_circle[1]), smallest_circle[2], Scalar(0, 255, 0), 2);
        // }

        int x = smallest_circle[0];
        int y = smallest_circle[1];

        circle_data.center = Point(x, y);
        circle_data.color = detected_color;
        if (config["debug_flag"].as<int>())
        {
            circle(img, circle_data.center, 5, Scalar(0, 0, 0), -1);
            imshow("Landmark_img", img);
        }
    } else
    {
        circle_data.center = Point(0, 0);

        if (config["debug_flag"].as<int>())
        {
            imshow("Landmark_img", img);
        }
    }

    if (config["debug_flag"].as<int>())
    {
        waitKey(1);
    }
}

Point2d Detector::calculateAverage(const std::deque<Point> &points)
{
    if (points.empty())
    {
        return Point2d(0.0, 0.0); // 如果队列为空，返回(0, 0)
    }

    double sum_x = 0.0;
    double sum_y = 0.0;

    for (const auto &point: points)
    {
        sum_x += point.x;
        sum_y += point.y;
    }

    double avg_x = sum_x / points.size();
    double avg_y = sum_y / points.size();

    return Point2d(avg_x, avg_y);
}

bool Detector::checkIfStationary(const std::deque<int> &x_positions, int threshold, int size)
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

int Detector::getMovementStatus(const Point &center, pair<int, int> range)
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
