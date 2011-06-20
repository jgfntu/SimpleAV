PROJECT(SIMPLEAV)

cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(LibAvCodec REQUIRED)
find_package(LibAvFormat REQUIRED)
find_package(LibAvUtil REQUIRED)
find_package(LibAvDevice REQUIRED)
find_package(LibSwscale REQUIRED)
find_package(SDL REQUIRED)
include_directories(
  ${AvCodec_INCLUDE_DIR} ${AvFormat_INCLUDE_DIR}
  ${AvUtil_INCLUDE_DIR} ${AvDevice_INCLUDE_DIR}
  ${Swscale_INCLUDE_DIR}
  ${SDL_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  )

file(GLOB SIMPLEAV_SOURCES
  ${PROJECT_SOURCE_DIR}/src/SimpleAV/SDL/*.c
  ${PROJECT_SOURCE_DIR}/src/SimpleAV/core/*.c
  ${PROJECT_SOURCE_DIR}/src/SimpleAV/core/util/*.c)
file(GLOB SIMPLEAV_HEADERS
  ${PROJECT_SOURCE_DIR}/include/SimpleAV/SDL/*.h
  ${PROJECT_SOURCE_DIR}/include/SimpleAV/core/*.h
  ${PROJECT_SOURCE_DIR}/include/SimpleAV/core/util/*.h)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_C_FLAGS "-g -Wall -O3")

add_executable(
  player2
  ${PROJECT_SOURCE_DIR}/player2.c
  ${SIMPLEAV_SOURCES}
  ${SIMPLEAV_HEADERS}
  )
target_link_libraries(
  player2
  ${AvCodec_LIBRARIES} ${AvFormat_LIBRARIES}
  ${AvUtil_LIBRARIES} ${AvDevice_LIBRARIES}
  ${Swscale_LIBRARIES}
  ${SDL_LIBRARY}
  m z pthread avcodec avformat avdevice avutil swscale
  )