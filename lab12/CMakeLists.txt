cmake_minimum_required(VERSION 3.14)
project(Lab12)

set(CMAKE_CXX_STANDARD 11)

include_directories(Z:/CSCI441/include)
link_directories(Z:/CSCI441/lib)

add_executable(Lab12 main.cpp)
target_link_libraries(Lab12 SOIL3 glew32.dll opengl32 glfw3 gdi32)