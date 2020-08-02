cmake_minimum_required(VERSION 3.14)
project(lab00a)

set(CMAKE_CXX_STANDARD 11)

include_directories(Z:/CSCI441/include)
link_directories(Z:/CSCI441/lib)

add_executable(lab00a main.cpp)
target_link_libraries(lab00a opengl32 glfw3 gdi32)