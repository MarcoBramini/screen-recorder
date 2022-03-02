cmake_minimum_required(VERSION 3.21)
project(screen_recorder)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_FLAGS "-fPIC")
set(CMAKE_C_FLAGS "-fPIC")

# PACKAGES
find_package(Threads REQUIRED)
find_package(fmt)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil
        )

# SOURCE FILES
set(SOURCES
        src/device_service/input_device.cpp
        src/device_service/input_device.h
        src/device_service/permission_status.h
        src/recording_service/recording_service_impl.cpp
        src/recording_service/recording_service_impl.h
        src/recording_service/recording_config.cpp
        src/recording_service/recording_config.h
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
        src/recording_service/process_chain/filter_ring.h
        src/recording_service/process_chain/process_context.h
        src/recording_service/process_chain/swscale_filter_ring.cpp
        src/recording_service/process_chain/swscale_filter_ring.h
        src/recording_service/process_chain/swresample_filter_ring.cpp
        src/recording_service/process_chain/swresample_filter_ring.h
        src/recording_service/process_chain/vfcrop_filter_ring.cpp
        src/recording_service/process_chain/vfcrop_filter_ring.h
        src/recording_service/packet_capturer/packet_capturer.cpp
        src/recording_service/packet_capturer/packet_capturer.h
        src/recording_service/ffmpeg_objects_deleter.cpp
        src/recording_service/ffmpeg_objects_deleter.h
        )
if (APPLE)
    set(SOURCES
            ${SOURCES}
            src/device_service/macos/avfoundation_bridge.mm
            src/device_service/macos/avfoundation_bridge.h
            src/device_service/macos/device_service_macos.cpp)
elseif (UNIX)
    set(SOURCES
            ${SOURCES}
            src/device_service/linux/device_service_linux.cpp)
elseif (WIN32)
    set(SOURCES
            ${SOURCES}
            src/device_service/windows/device_service_windows.cpp)
endif ()

# LIBRARY
add_library(screen_recorder STATIC ${SOURCES})

target_include_directories(screen_recorder INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# LINK DEPENDENCIES
target_link_libraries(screen_recorder PUBLIC PkgConfig::LIBAV)
target_link_libraries(screen_recorder PUBLIC fmt::fmt)
target_link_libraries(screen_recorder PUBLIC Threads::Threads)

if (APPLE)
    target_link_libraries(screen_recorder PUBLIC
            "-framework AVFoundation"
            "-framework CoreGraphics"
            "-framework CoreMotion"
            "-framework Foundation"
            "-framework MediaPlayer"
            "-framework QuartzCore"
            )
elseif (UNIX)
    target_link_libraries(screen_recorder PUBLIC X11)
    target_link_libraries(screen_recorder PUBLIC Xrandr)
endif ()