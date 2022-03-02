cmake_minimum_required(VERSION 3.16)

project(qt_screen_recorder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 COMPONENTS Quick QuickControls2 Gui Widgets REQUIRED)

qt_add_executable(appqt_screen_recorder
    main.cpp
    qml.qrc
    components/components.qrc
    src/backend.cpp
    src/backend.h
)

target_include_directories(appqt_screen_recorder PRIVATE src)

qt_add_qml_module(appqt_screen_recorder
    URI qt_screen_recorder
    VERSION 1.0
    QML_FILES main.qml
)

set_target_properties(appqt_screen_recorder PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.pds.screenrecorder
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(appqt_screen_recorder
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(appqt_screen_recorder
    PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Widgets Qt6::Gui screen_recorder)
