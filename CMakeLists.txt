cmake_minimum_required(VERSION 2.6)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

Project(BooxOpensource CXX C)
enable_testing()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/libs)
SET(TEST_OUTPUT_PATH ${PROJECT_BINARY_DIR}/unittests)
SET(QT_PLUGINS_OUTPUT_PATH ${PROJECT_BINARY_DIR}/plugins)


# Global definitions
SET(TTS_LIB "")
SET(ADD_LIB "")
SET(SAMPLE_ROOT "${PROJECT_SOURCE_DIR}/samples")
ADD_DEFINITIONS(-DSAMPLE_ROOT="${SAMPLE_ROOT}")
ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE)

include(enable_qt)
include(onyx_test)
include(strict_warning)
include(tc)
include(FindIconv)
include(misc)

find_package(Qt4 4.5.2 COMPONENTS QtCore QtGui QtXml QtSql QtWebKit QtNetwork
  QtTest QtDBus REQUIRED)
include(${QT_USE_FILE})

include_directories(BEFORE
  ${CMAKE_SOURCE_DIR}/sdk/include
  ${CMAKE_SOURCE_DIR}/code/
  ${CMAKE_SOURCE_DIR}/third_party
  ${CMAKE_SOURCE_DIR}/third_party/gtest/include
  ${CMAKE_SOURCE_DIR}/third_party/gmock/include
)

if(BUILD_FOR_ARM)
  include(arm_toolchain)
  use_arm_toolchain()
  # Maximize window.
  add_definitions(-DSCREEN_WIDTH=-1 -DSCREEN_HEIGHT=-1 -DBUILD_FOR_ARM)
  add_definitions(-DSHARE_ROOT="/usr/share")
  link_directories(${CMAKE_SOURCE_DIR}/sdk/lib/arm)
else(BUILD_FOR_ARM)
  # Use 600x800 to approximate device screen on PC.
  add_definitions(-DSCREEN_WIDTH=600)
  add_definitions(-DSCREEN_HEIGHT=800)
  add_definitions(-DSHARE_ROOT="")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  link_directories(${CMAKE_SOURCE_DIR}/sdk/lib/x86)
endif(BUILD_FOR_ARM)

set(ONYX_LIBS onyx_sys onyx_ui onyx_wifi onyx_wpa onyx_screen onyx_data onyx_cms)
ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE)
IF (WIN32)
ADD_DEFINITIONS(-DCONFIG_NATIVE_WINDOWS)
ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE_NAMED_PIPE)
ELSE (WIN32)
ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE_UNIX)
ENDIF (WIN32)

add_subdirectory(code/src)
add_subdirectory(third_party)
