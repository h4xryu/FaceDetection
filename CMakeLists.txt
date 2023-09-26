cmake_minimum_required(VERSION 3.22.1)
project(video_test)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(video_test video.cpp)
target_link_libraries(video_test ${OpenCV_LIBS})

