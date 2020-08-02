cmake_minimum_required(VERSION 3.14)
project(lab02)

set(CMAKE_CXX_STANDARD 11)

include_directories(Z:/CSCI441/include)
link_directories(Z:/CSCI441/lib)

add_executable(lab02 main.cpp)
target_link_libraries(lab02 opengl32 glfw3 gdi32)