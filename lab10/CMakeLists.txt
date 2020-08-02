cmake_minimum_required(VERSION 3.14)
project(Lab10)

set(CMAKE_CXX_STANDARD 11)

include_directories(Z:/CSCI441/include)
link_directories(Z:/CSCI441/lib)

add_executable(Lab10 main.cpp)
target_link_libraries(Lab10 SOIL3 glew32.dll opengl32 glfw3 gdi32)