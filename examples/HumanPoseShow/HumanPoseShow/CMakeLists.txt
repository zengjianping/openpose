cmake_minimum_required(VERSION 3.5)

project(HumanPoseShow VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
set(OpenPose_DIR "../../../install/lib/OpenPose/")
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenPose REQUIRED)

find_package(PkgConfig REQUIRED)
find_package(jsoncpp REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

include_directories(
    ${OpenPose_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

link_directories(
    ${JSONCPP_LIBRARY_DIRS}
)

set(TS_FILES HumanPoseShow_zh_CN.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        CommonUtils.h
        CommonUtils.cpp
        VideoItemWidget.h
        VideoItemWidget.cpp
        VideoGroupWidget.h
        VideoGroupWidget.cpp
        HumanPoseProcessor.h
        HumanPoseProcessor.cpp
        StereoPoseRender.hpp
        StereoPoseRender.cpp
        PoseRenderWidget.h
        PoseRenderWidget.cpp
        LayoutVideoWidget.h
        LayoutVideoWidget.cpp
        LayoutPageWidget.h
        LayoutPageWidget.cpp
        DialogOutputConfig.h
        DialogOutputConfig.cpp
        DialogOutputConfig.ui
        DialogInputConfig.h
        DialogInputConfig.cpp
        DialogInputConfig.ui
        DialogAlgoConfig.h
        DialogAlgoConfig.cpp
        DialogAlgoConfig.ui
        CameraCalibWidget.h
        CameraCalibWidget.cpp
        CameraCalibWidget.ui
        DialogTaskList.h
        DialogTaskList.cpp
        DialogTaskList.ui
        capturethread.h
        capturethread.cpp
        MindCameraConfig.h
        MindCameraConfig.cpp
        MindCameraConfig.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HumanPoseShow
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET HumanPoseShow APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(HumanPoseShow SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(HumanPoseShow
            ${PROJECT_SOURCES}
            HumanPoseShow.qrc
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_directories(HumanPoseShow PRIVATE "../../../install/lib/")
target_link_libraries(HumanPoseShow PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    ${OpenPose_LIBS} caffe Eigen3::Eigen MVSDK ${JSONCPP_LIBRARIES})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.HumanPoseShow)
endif()
set_target_properties(HumanPoseShow PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS HumanPoseShow
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(HumanPoseShow)
endif()
