cmake_minimum_required(VERSION 3.14)
project(lab04)

set(CMAKE_CXX_STANDARD 11)

include_directories(Z:/CSCI441/include)
link_directories(Z:/CSCI441/lib)

add_executable(lab04 main.cpp)
target_link_libraries(lab04 opengl32 glfw3 gdi32)