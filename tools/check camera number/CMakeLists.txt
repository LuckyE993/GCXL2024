cmake_minimum_required(VERSION 3.10)

# 项目名称
project(Dual_Camera_Display)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 寻找OpenCV库
find_package(OpenCV REQUIRED)

# 添加可执行文件
add_executable(check_camera_number main.cpp)

# 链接OpenCV库
target_link_libraries(check_camera_number ${OpenCV_LIBS})
