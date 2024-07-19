
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

#set(Qt5_DIR "/home/zengjianping/Qt/5.15.1/gcc_64/lib/cmake/Qt5/")
#set(Qt5_DIR "/usr/lib/x86_64-linux-gnu/cmake/Qt5/")
find_package(Qt5 COMPONENTS Widgets REQUIRED)

find_package(PkgConfig REQUIRED)
find_package(jsoncpp REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

find_package(Boost COMPONENTS system filesystem REQUIRED)

include_directories(
    ${JSONCPP_INCLUDE_DIRS}
)

link_directories(
    ${JSONCPP_LIBRARY_DIRS}
)

set(EXE_NAME "HumanPoseShow.bin")
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
  HumanPoseShow/PoseRenderWidget.h
  HumanPoseShow/PoseRenderWidget.cpp
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
  HumanPoseShow/DialogTaskList.h
  HumanPoseShow/DialogTaskList.cpp
  HumanPoseShow/DialogTaskList.ui
  HumanPoseShow/MindCameraConfig.h
  HumanPoseShow/MindCameraConfig.cpp
  HumanPoseShow/MindCameraConfig.ui
  HumanPoseShow/capturethread.h
  HumanPoseShow/capturethread.cpp
)

target_link_libraries(${EXE_NAME}
  Qt5::Widgets
  openpose
  crypto++
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${JSONCPP_LIBRARIES}
  ${examples_3rdparty_libraries}
)

install(TARGETS ${EXE_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/datas/pose_tasks
  DESTINATION datas/)

  install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/models/pose/body_25
    DESTINATION models/pose/)
  
  install(
  PROGRAMS ${PROJECT_SOURCE_DIR}/scripts/human_pose_show.sh
  DESTINATION scripts/
)

