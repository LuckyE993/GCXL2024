cmake_minimum_required(VERSION 3.10)
project(host)

find_package(OpenCV REQUIRED)

include_directories(
        ${CMAKE_SOURCE_DIR}/inc
        ${OpenCV_INCLUDE_DIRS}
)

add_executable(host
        src/usart.cpp
        main.cpp
        src/QRcode.cpp
        inc/QRcode.h
        src/Camera.cpp
        inc/Camera.h

)

find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)


target_link_libraries(host
        ${CMAKE_THREAD_LIBS_INIT}
        yaml-cpp
        ${OpenCV_LIBS}
)


