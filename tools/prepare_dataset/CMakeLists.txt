cmake_minimum_required(VERSION 3.10)
project(prepare_dataset)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)

add_executable(prepare_dataset prepare_dataset.cpp)

target_link_libraries(prepare_dataset ${OpenCV_LIBS})
