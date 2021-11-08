cmake_minimum_required(VERSION 3.16.3)
project(pds_screen_recording)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include_directories("/usr/local/include")
link_directories("/usr/local/lib")

find_package(Threads REQUIRED)

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
        src/recording_service/recording_service.cpp
        src/recording_service/recording_service.h
        src/recording_service/recording_utils.cpp
        src/recording_service/device_context.cpp
        src/recording_service/device_context.h
        src/recording_service/error.cpp
        src/recording_service/error.h
        src/recording_service/process_chain/process_chain.cpp
        src/recording_service/process_chain/process_chain.h
        src/recording_service/process_chain/decoder_ring.cpp
        src/recording_service/process_chain/decoder_ring.h
        src/recording_service/process_chain/muxer_ring.cpp
        src/recording_service/process_chain/muxer_ring.h
        src/recording_service/process_chain/encoder_ring.cpp
        src/recording_service/process_chain/encoder_ring.h
        src/recording_service/process_chain/filter_ring.cpp
        src/recording_service/process_chain/filter_ring.h
        ${FILES}
        src/recording_service/process_chain/process_context.cpp
        src/recording_service/process_chain/process_context.h
        src/recording_service/process_chain/swscale_filter_ring.cpp
        src/recording_service/process_chain/swscale_filter_ring.h
        src/recording_service/process_chain/swresample_filter_ring.cpp
        src/recording_service/process_chain/swresample_filter_ring.h)


target_link_libraries(pds_screen_recording avdevice)
target_link_libraries(pds_screen_recording avformat)
target_link_libraries(pds_screen_recording avutil)
target_link_libraries(pds_screen_recording avcodec)
target_link_libraries(pds_screen_recording swresample)
target_link_libraries(pds_screen_recording swscale)
target_link_libraries(pds_screen_recording fmt)
target_link_libraries(pds_screen_recording pthread)

if (${APPLE})
    target_link_libraries(pds_screen_recording
            "-framework AVFoundation"
            "-framework CoreGraphics"
            "-framework CoreMotion"
            "-framework Foundation"
            "-framework MediaPlayer"
            "-framework QuartzCore"
            )
elseif (${UNIX})
    target_link_libraries(pds_screen_recording X11)
    target_link_libraries(pds_screen_recording Xrandr)
endif ()