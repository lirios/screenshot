cmake_minimum_required(VERSION 3.10.0)

project("ScreenShot"
    VERSION "0.9.0"
    DESCRIPTION "Capture the screen of your session"
    LANGUAGES CXX C
)

## Shared macros and functions:
if(NOT LIRI_LOCAL_ECM)
    find_package(LiriCMakeShared "2.0.0" REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH "${LCS_MODULE_PATH}")
endif()

## Set minimum versions required:
set(QT_MIN_VERSION "5.10.0")

## Liri specific setup common for all modules:
include(LiriSetup)

## Only build on the appropriate platforms.
if(NOT UNIX OR ANDROID)
    message(NOTICE "Skipping the build as the condition \"UNIX OR ANDROID\" is not met.")
    return()
endif()

## Find Qt 5:
find_package(Qt5 "${QT_MIN_VERSION}"
    CONFIG REQUIRED
    COMPONENTS
        Core
        DBus
        Gui
        Widgets
        Qml
        Quick
        QuickControls2
        LinguistTools
        WaylandClient
)

## Add subdirectories:
add_subdirectory(src/screenshot)
