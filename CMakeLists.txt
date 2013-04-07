cmake_minimum_required(VERSION 2.8)

project(koku-yuv4mpeg-3D)
ADD_EXECUTABLE(koku-yuv4mpeg-3D main.cpp)
include_directories("${PROJECT_SOURCE_DIR}/extern/lib")
set(CMAKE_CXX_FLAGS "-std=c++0x -ldl -lrt -lGL")
add_subdirectory(extern)
target_link_libraries(koku-yuv4mpeg-3D pthread koku-opengl-framework)
