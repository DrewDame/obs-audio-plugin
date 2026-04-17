cmake_minimum_required(VERSION 3.20)

project(obs-audio-recorder VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point this at your OBS install or build dir via -DLIBOBS_INCLUDE_DIR etc.
find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_library(obs-audio-recorder MODULE
    src/plugin-main.cpp
    src/audio-recorder.cpp
    src/audio-recorder-dock.cpp
)

target_link_libraries(obs-audio-recorder
    PRIVATE
        OBS::libobs
        OBS::obs-frontend-api
        Qt6::Widgets
)

target_include_directories(obs-audio-recorder PRIVATE src)

# Copy plugin into OBS plugin directory after build (optional helper)
if(DEFINED OBS_PLUGIN_DESTINATION)
    install(TARGETS obs-audio-recorder
        LIBRARY DESTINATION "${OBS_PLUGIN_DESTINATION}"
    )
    install(DIRECTORY data/
        DESTINATION "${OBS_PLUGIN_DESTINATION}/../data/obs-plugins/obs-audio-recorder"
    )
endif()
