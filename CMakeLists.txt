cmake_minimum_required(VERSION 2.8.4)

project(Profiler)

set(version 0.3.0)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

OPTION(LINK_STATICALLY "Specify option if you plan to link the profiler statically" OFF)

set(PROJECT_SO_VERSION ${version})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmakeModules)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)

include(SetVSTargetProperties)

set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
set(INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/install/${CMAKE_PROJECT_NAME}/${version})

add_subdirectory(thirdparty)
include_directories(${BASE_DIRECTORY}/src)
add_subdirectory(src)

set(gtest_VERSION 1.6.0)
add_subdirectory(test)

