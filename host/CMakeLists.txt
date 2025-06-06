cmake_minimum_required(VERSION 3.10)
project(host)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZBAR REQUIRED zbar)

# 包含头文件目录
include_directories(
        ${CMAKE_SOURCE_DIR}/inc
        ${OpenCV_INCLUDE_DIRS}
        ${ZBAR_INCLUDE_DIRS}
)

# 定义可执行文件和源文件

add_executable(host
        inc/usart.hpp
        src/usart.cpp
        inc/Camera.h
        src/Camera.cpp
        inc/QRcode.h
        src/QRcode.cpp
        inc/Command.h
        src/Command.cpp
        inc/Detector.h
        src/Detector.cpp
        main.cpp

)

# 设置头文件目录
target_include_directories(host PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZBAR_INCLUDE_DIRS})

# 链接库文件
target_link_libraries(host
        ${CMAKE_THREAD_LIBS_INIT}       # 链接线程库
        yaml-cpp::yaml-cpp              # 链接 yaml-cpp 库
        ${OpenCV_LIBS}                  # 链接 OpenCV 库
        ${ZBAR_LIBRARIES}               # 链接 ZBar 库 (修正了 ZBAR_INCLUDE_DIRS 为 ZBAR_LIBRARIES)
)


add_executable(show_QR_info src/show_QR_info.cpp)
target_include_directories(show_QR_info PRIVATE ${OpenCV_INCLUDE_DIRS} ${ZBAR_INCLUDE_DIRS})
target_link_libraries(show_QR_info
        ${OpenCV_LIBS}                  # 链接 OpenCV 库
)