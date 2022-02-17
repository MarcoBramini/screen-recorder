cmake_minimum_required(VERSION 3.16.3)
project(cmd_screen_recorder)

set(CMAKE_CXX_STANDARD 20)

add_executable(cmd_screen_recorder main.cpp)

target_link_libraries(cmd_screen_recorder LINK_PUBLIC screen_recorder)
