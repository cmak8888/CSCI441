cmake_minimum_required(VERSION 3.14)
project(Lab11)

set(CMAKE_CXX_STANDARD 11)

include_directories(Z:/CSCI441/include)
link_directories(Z:/CSCI441/lib)

add_executable(Lab11 main.cpp include/Marble.h src/Marble.cpp)
target_link_libraries(Lab11 SOIL3 glew32.dll opengl32 glfw3 gdi32)