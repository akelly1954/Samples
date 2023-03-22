cmake_minimum_required(VERSION 3.5)

project(VideoCapturePlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package( Threads )

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ldl")

#######################################################################################
#
# TODO: The definitions below should be included from $ENV{SampleRoot_DIR}/source/cmake/QtApps.cmake)

set (EnetUtil_HEADERS         "$ENV{SampleRoot_DIR}/source/EnetUtil/include" )
set (Util_HEADERS             "$ENV{SampleRoot_DIR}/source/Util/include")
set (VideoPlugin_V4L2_HEADERS "$ENV{SampleRoot_DIR}/source/Video/include" )    ##########   $ENV{OpenCV_INCLUDE_DIRS} )
set (Video_HEADERS            "$ENV{SampleRoot_DIR}/source/Video/include" ) ########   $ENV{OpenCV_INCLUDE_DIRS} )

set( LoggerCpp_BASE           "$ENV{SampleRoot_DIR}/source/3rdparty/LoggerCpp" )
set( LoggerCppSource_DIR      "${LoggerCpp_BASE}/SRombauts-LoggerCpp-a0868a8-modified" )
set( LoggerCpp_HEADERS        "${LoggerCppSource_DIR}/include" )
set( LoggerCpp_LIB            "${LoggerCpp_BASE}/build/libLoggerCpp.a")

set( JsonCpp_BASE             "$ENV{SampleRoot_DIR}/source/3rdparty/JsonCpp" )
set( JsonCppSource_DIR        "${JsonCpp_BASE}/JsonCpp-8190e06-2022-07-15/jsoncpp" )
set( JsonCpp_HEADERS          "${JsonCppSource_DIR}/include" )
set( JsonCpp_LIB              "${JsonCpp_BASE}/build/lib/libjsoncpp.a")

set( LoggerCpp_LIBTYPE "STATIC")
set( JsonCpp_LIBTYPE "SHARED")

set (EnetUtil_LIB "$ENV{SampleRoot_DIR}/build/lib/libEnetUtil.so")
set (Enet_LIBTYPE "SHARED")
set (Util_LIB "$ENV{SampleRoot_DIR}/build/lib/libUtil.so")
set (Util_LIBTYPE "SHARED")
set (Video_LIB "$ENV{SampleRoot_DIR}/build/lib/libVideo.so")
set (Video_LIBTYPE "SHARED")
set (VideoPlugin_V4L2_LIB "$ENV{SampleRoot_DIR}/build/lib/libVideoPlugin_V4L2.so")

include_directories( ${CMAKE_SOURCE_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/tools )
include_directories( ${EnetUtil_HEADERS} )
include_directories( ${Util_HEADERS} )
include_directories( ${VideoPlugin_V4L2_HEADERS} )
include_directories( ${Video_HEADERS} )
include_directories( ${LoggerCpp_HEADERS} )
include_directories( ${JsonCpp_HEADERS} )


set(PROJECT_SOURCES
        main.cpp
        stream2qt_video_capture.hpp
        stream2qt_video_capture.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        protovideoplayer.hpp
        protovideoplayer.cpp
        tools/control_main.cpp
        tools/control_main.hpp
        tools/logger_tools.hpp
        tools/parse_tools.hpp
        tools/config_tools.cpp
        tools/config_tools.hpp
        tools/parse_tools.cpp
        tools/logger_tools.cpp
        tools/nonqt_util.cpp
        tools/nonqt_util.hpp
        tools/vidstream_profiler_thread.hpp
        tools/vidstream_profiler_thread.cpp
        resources.qrc
        video_capture_player.json
)
# videowidget.ui
# videowidget.hpp
# videowidget.cpp
# video_stream_buffers.hpp
# video_stream_buffers.cpp

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VideoCapturePlayer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET VideoCapturePlayer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(VideoCapturePlayer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(VideoCapturePlayer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(
            VideoCapturePlayer PRIVATE 
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Multimedia
            Qt${QT_VERSION_MAJOR}::MultimediaWidgets
            ${Video_LIB}
            ${EnetUtil_LIB}
            ${Util_LIB}
            ${LoggerCpp_LIB}
            ${JsonCpp_LIB}
            ${CMAKE_THREAD_LIBS_INIT} ${LINKOPTIONS}
        )

set_target_properties(VideoCapturePlayer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS VideoCapturePlayer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VideoCapturePlayer)
endif()
