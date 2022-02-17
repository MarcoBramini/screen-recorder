cmake_minimum_required(VERSION 3.16)

project(qt_screen_recorder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 COMPONENTS Quick QuickControls2 Gui REQUIRED)

qt_add_executable(appqt_screen_recorder
    main.cpp
    qml.qrc
    components/components.qrc
)

qt_add_qml_module(appqt_screen_recorder
    URI qt_screen_recorder
    VERSION 1.0
    QML_FILES main.qml
)

add_subdirectory(backend)

set_target_properties(appqt_screen_recorder PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(appqt_screen_recorder
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(appqt_screen_recorder
    PRIVATE Qt6::Quick Qt6::QuickControls2 backend)
