cmake_minimum_required(VERSION 3.5)

project(MultimediaPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt6 COMPONENTS Multimedia REQUIRED)

set(TS_FILES MultimediaPlayer_zh_CN.ts)

# 导入视频解码模块进行编译
set(DECODE_MODULE_DIR multimedia_decode_module)
set(ROOT_DIR MultimediaPlayer)
FILE(GLOB DECODE_MODULE_HEADER "${DECODE_MODULE_DIR}/*.hpp" "${DECODE_MODULE_DIR}/*.h")
FILE(GLOB DECODE_MODULE_SOURCE "${DECODE_MODULE_DIR}/*.cpp" "${DECODE_MODULE_DIR}/*.c")
source_group("${DECODE_MODULE_DIR}/Header" FILES ${DECODE_MODULE_HEADER})
source_group("${DECODE_MODULE_DIR}/Source" FILES ${DECODE_MODULE_SOURCE})

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        ${DECODE_MODULE_HEADER}
        ${DECODE_MODULE_SOURCE}
        ${TS_FILES}
)

include_directories(${PROJECT_SOURCE_DIR}/ffmpeg/include)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/ffmpeg/lib)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MultimediaPlayer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MultimediaPlayer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(MultimediaPlayer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MultimediaPlayer
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(MultimediaPlayer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(MultimediaPlayer PUBLIC Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(MultimediaPlayer PUBLIC avutil avcodec avfilter avdevice avformat postproc swresample)

set_target_properties(MultimediaPlayer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MultimediaPlayer)
endif()
