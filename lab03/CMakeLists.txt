cmake_minimum_required(VERSION 3.14)
project(lab03)

set(CMAKE_CXX_STANDARD 11)

include_directories(Z:/CSCI441/include)
link_directories(Z:/CSCI441/lib)

add_executable(lab03 main.cpp)
target_link_libraries(lab03 opengl32 glfw3 gdi32)