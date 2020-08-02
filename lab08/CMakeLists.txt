cmake_minimum_required(VERSION 3.14)
project(Lab08)

set(CMAKE_CXX_STANDARD 11)

include_directories(Z:/CSCI441/include)
include_directories(./include)
link_directories(Z:/CSCI441/lib)
link_directories(./lib)


add_executable(Lab08 main.cpp src/md5mesh.cpp src/md5anim.cpp)
target_link_libraries(Lab08 lab08 SOIL3 glew32.dll opengl32 glfw3 gdi32)