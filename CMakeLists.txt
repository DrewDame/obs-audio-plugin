cmake_minimum_required(VERSION 3.20)

project(obs-audio-recorder VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Prefer standard OBS CMake packages when available.
find_package(libobs QUIET)
find_package(obs-frontend-api QUIET)

# Fallback for local OBS source/build trees where package config files are not generated.
if(NOT TARGET OBS::libobs OR NOT TARGET OBS::obs-frontend-api)
    set(OBS_SOURCE_DIR "" CACHE PATH "Path to OBS source tree")
    set(OBS_BUILD_DIR "" CACHE PATH "Path to OBS CMake build directory")

    if(OBS_SOURCE_DIR STREQUAL "" OR OBS_BUILD_DIR STREQUAL "")
        message(FATAL_ERROR
            "Could not find OBS CMake packages. Set OBS_SOURCE_DIR and OBS_BUILD_DIR "
            "to your local OBS source and build directories."
        )
    endif()

    if(NOT TARGET OBS::libobs)
        add_library(OBS::libobs SHARED IMPORTED)
        set_target_properties(OBS::libobs PROPERTIES
            IMPORTED_IMPLIB "${OBS_BUILD_DIR}/libobs/Release/obs.lib"
            IMPORTED_LOCATION "${OBS_BUILD_DIR}/rundir/Release/bin/64bit/obs.dll"
            INTERFACE_INCLUDE_DIRECTORIES "${OBS_SOURCE_DIR}/libobs;${OBS_BUILD_DIR}/libobs;${OBS_BUILD_DIR}/config"
        )
    endif()

    if(NOT TARGET OBS::obs-frontend-api)
        add_library(OBS::obs-frontend-api SHARED IMPORTED)
        set_target_properties(OBS::obs-frontend-api PROPERTIES
            IMPORTED_IMPLIB "${OBS_BUILD_DIR}/frontend/api/Release/obs-frontend-api.lib"
            IMPORTED_LOCATION "${OBS_BUILD_DIR}/rundir/Release/bin/64bit/obs-frontend-api.dll"
            INTERFACE_INCLUDE_DIRECTORIES "${OBS_SOURCE_DIR}/frontend/api"
        )
    endif()
endif()

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

# Installer/package layout for plugin distribution.
install(TARGETS obs-audio-recorder
    RUNTIME DESTINATION "obs-plugins/64bit"
    LIBRARY DESTINATION "obs-plugins/64bit"
)
install(DIRECTORY data/
    DESTINATION "data/obs-plugins/obs-audio-recorder"
)

set(CPACK_PACKAGE_NAME "obs-audio-recorder")
set(CPACK_PACKAGE_VENDOR "obs-audio-recorder")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-windows-x64")
set(CPACK_GENERATOR "ZIP;NSIS")
# Default NSIS install location to the OBS app folder so plugin files
# land under <OBS>/obs-plugins and <OBS>/data.
set(CPACK_PACKAGE_INSTALL_DIRECTORY "obs-studio")
# Must differ from OBS Studio's own uninstaller registration and files:
# default CPack uses HKLM\...\Uninstall\{subdir} and InstDir\Uninstall.exe
# — both collide with a normal OBS install (same folder + same "obs-studio" key).
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "obs-audio-recorder-plugin")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall-obs-audio-recorder")
set(CPACK_NSIS_DISPLAY_NAME "OBS Audio Recorder Plugin")
set(CPACK_NSIS_PACKAGE_NAME "OBS Audio Recorder Plugin")
include(CPack)
