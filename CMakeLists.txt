#Project
project(cork)
cmake_minimum_required(VERSION 2.8)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(TISCAMERA_DIR ~/tiscamera)

#Check modules
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
pkg_check_modules(TCAMLIB tcam)

#Includes
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${TISCAMERA_DIR}/examples/cpp/common  ${GSTREAMER_INCLUDE_DIRS} ${TCAM_INCLUDE_DIRS}  ${OpenCV_INCLUDE_DIRS})
add_definitions(${GSTREAMER_CFLAGS_OTHER}) 

#Build
add_executable(cork main.cpp)
target_link_libraries(cork ${OpenCV_LIBS} ${TCAMLIB_LIBRARIES} ${GSTREAMER_LIBRARIES})
