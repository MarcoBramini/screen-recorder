qt6_add_qml_module(backend
    URI Backend
    VERSION 1.0
    SOURCES
        backend.cpp backend.h
)

target_include_directories(backendplugin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(backend LINK_PUBLIC screen_recorder Qt6::Gui)
