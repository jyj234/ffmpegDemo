cmake_minimum_required(VERSION 3.16)

project(ffmpegDemo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OPENCV_LIB_DIR "D:/opencv/build/x64/vc15/lib")
set(OPENCV_INCLUDE_DIR "D:/opencv/build/include")
set(FFMPEG_LIB_DIR "D:/ffmpegLib/lib")
set(FFMPEG_INCLUDE_DIR "D:/ffmpegLib/include")
file(GLOB OPENCV_LIBS ${OPENCV_LIB_DIR}/opencv_world452.lib)
file(GLOB OPENCV_H ${OPENCV_INCLUDE_DIR}/opencv2/*.hpp)
file(GLOB FFMPEG_H ${FFMPEG_INCLUDE_DIR}/*.h)
file(GLOB FFMPEG_LIBS ${FFMPEG_LIB_DIR}/*.lib)
include_directories(${OPENCV_INCLUDE_DIR})
include_directories(${FFMPEG_INCLUDE_DIR})
find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
qt_standard_project_setup()

qt_add_executable(appffmpegDemo
    main.cpp
)

qt_add_qml_module(appffmpegDemo
    URI ffmpegDemo
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES frameprovider.h frameprovider.cpp
    RESOURCES
    SOURCES mux.cpp mux.h
    SOURCES decoder.h decoder.cpp

)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appffmpegDemo PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appffmpegDemo
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appffmpegDemo
    PRIVATE Qt6::Quick
    PRIVATE ${OPENCV_LIBS}
    PRIVATE ${FFMPEG_LIBS}
    PRIVATE Qt6::Multimedia

)

include(GNUInstallDirs)
install(TARGETS appffmpegDemo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)