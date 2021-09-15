cmake_minimum_required(VERSION 3.17)
project(pds_screen_recording)

set(CMAKE_CXX_STANDARD 20)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(pds_screen_recording main.cpp)

target_link_libraries(pds_screen_recording avdevice)
target_link_libraries(pds_screen_recording avformat)