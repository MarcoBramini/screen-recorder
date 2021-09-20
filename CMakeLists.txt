cmake_minimum_required(VERSION 3.17)
project(pds_screen_recording)

set(CMAKE_CXX_STANDARD 20)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

if (${APPLE})
    set(FILES
            src/device_service/macos/avfoundation_bridge.mm
            src/device_service/macos/avfoundation_bridge.h
            src/device_service/macos/device_service_macos.cpp)
elseif (${UNIX})
    set(FILES
            src/device_service/linux/device_service_linux.cpp)
elseif (${WIN32})
    set(FILES
            src/device_service/windows/device_service_windows.cpp)
endif ()

add_executable(pds_screen_recording
        main.cpp
        src/device_service/input_device.h
        src/device_service/device_service.h
        ${FILES})


target_link_libraries(pds_screen_recording avdevice)
target_link_libraries(pds_screen_recording avformat)
target_link_libraries(pds_screen_recording avutil)

if(${APPLE})
    target_link_libraries(pds_screen_recording
            "-framework AVFoundation"
            "-framework CoreGraphics"
            "-framework CoreMotion"
            "-framework Foundation"
            "-framework MediaPlayer"
            "-framework QuartzCore"
            )
elseif(${UNIX})
    target_link_libraries(pds_screen_recording X11)
    target_link_libraries(pds_screen_recording Xrandr)
endif()