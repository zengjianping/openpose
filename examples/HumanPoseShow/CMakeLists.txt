
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(Qt5_DIR "/home/zengjianping/Qt/5.15.1/gcc_64/lib/cmake/Qt5/")
find_package(Qt5 COMPONENTS Widgets REQUIRED)

find_package(PkgConfig REQUIRED)
find_package(jsoncpp REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

include_directories(
    ${JSONCPP_INCLUDE_DIRS}
)

link_directories(
    ${JSONCPP_LIBRARY_DIRS}
)

set(EXE_NAME "HumanPoseShow")
message(STATUS "Adding Example ${EXE_NAME}")

add_executable(${EXE_NAME}
  HumanPoseShow/HumanPoseShow.qrc
  HumanPoseShow/HumanPoseShow_zh_CN.ts
  HumanPoseShow/main.cpp
  HumanPoseShow/mainwindow.h
  HumanPoseShow/mainwindow.cpp
  HumanPoseShow/CommonUtils.h
  HumanPoseShow/CommonUtils.cpp
  HumanPoseShow/VideoItemWidget.h
  HumanPoseShow/VideoItemWidget.cpp
  HumanPoseShow/VideoGroupWidget.h
  HumanPoseShow/VideoGroupWidget.cpp
  HumanPoseShow/HumanPoseProcessor.h
  HumanPoseShow/HumanPoseProcessor.cpp
  HumanPoseShow/StereoPoseRender.hpp
  HumanPoseShow/StereoPoseRender.cpp
  HumanPoseShow/LayoutVideoWidget.h
  HumanPoseShow/LayoutVideoWidget.cpp
  HumanPoseShow/LayoutPageWidget.h
  HumanPoseShow/LayoutPageWidget.cpp
  HumanPoseShow/DialogOutputConfig.h
  HumanPoseShow/DialogOutputConfig.cpp
  HumanPoseShow/DialogOutputConfig.ui
  HumanPoseShow/DialogInputConfig.h
  HumanPoseShow/DialogInputConfig.cpp
  HumanPoseShow/DialogInputConfig.ui
  HumanPoseShow/DialogAlgoConfig.h
  HumanPoseShow/DialogAlgoConfig.cpp
  HumanPoseShow/DialogAlgoConfig.ui
  HumanPoseShow/CameraCalibWidget.h
  HumanPoseShow/CameraCalibWidget.cpp
  HumanPoseShow/CameraCalibWidget.ui
  HumanPoseShow/MindCameraConfig.h
  HumanPoseShow/MindCameraConfig.cpp
  HumanPoseShow/MindCameraConfig.ui
  HumanPoseShow/capturethread.h
  HumanPoseShow/capturethread.cpp
)

target_link_libraries(${EXE_NAME}
  Qt5::Widgets
  openpose
  ${JSONCPP_LIBRARIES}
  ${examples_3rdparty_libraries}
)

