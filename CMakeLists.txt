PROJECT(SimpleAV)

set(SimpleAV_MAJOR 0)
set(SimpleAV_MINOR 1)
set(SimpleAV_PATCH 1)

set(SimpleAV_VERSION ${SimpleAV_MAJOR}.${SimpleAV_MINOR}.${SimpleAV_PATCH})
set(SimpleAV_DESCRIPTION "SimpleAV is a simple decoding library.")
set(SimpleAV_HOMEPAGE "https://github.com/wecing/SimpleAV")
# EXENAME and PACKAGENAME?

cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(LibAV REQUIRED)
find_package(BZip2)
find_package(ZLIB)
find_package(SDL REQUIRED) # FIXME: This should be optional.

include_directories(${LIBAV_INCLUDE_DIRS})
include_directories(${SDL_INCLUDE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${SimpleAV_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH ${SimpleAV_SOURCE_DIR})

# include_directories?

set(CMAKE_C_FLAGS "-W -Wall -g -O2") # include predefined CMAKE_C_FLAGS value?

# ignore creating packages for now.

## file(GLOB SimpleAV_SOURCES ./*.c)
## file(GLOB SimpleAV_HEADERS ./*.h)
## 
## add_executable()

add_library(SimpleAV SimpleAV.c SAQueue.c SAMutex.c) # FIXME: what about SAQueue.c and SAMutex.c?
# target_link_libraries(SimpleAV avcodec avformat swscale avutil) # FIXME: is this necessary?
target_link_libraries(SimpleAV avcodec avformat avdevice avutil swscale avfilter ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES})

add_executable(saplayer-old saplayer-old.c) # FIXME: build it as a single executable, which doesn't need the shared/dymanic lib files?
# target_link_libraries(saplayer-old libavcodec libavformat libswscale libavutil SDL SimpleAV)
# target_link_libraries(saplayer-old avcodec avformat swscale avutil ${SDL_LIBRARY} SimpleAV)
target_link_libraries(saplayer-old avcodec avformat avdevice avutil swscale avfilter ${SDL_LIBRARY} SimpleAV ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES})

install(FILES SimpleAV.h DESTINATION include)
install(FILES libSimpleAV.a DESTINATION lib)
install(FILES SimpleAV.pc DESTINATION lib/pkgconfig)
# Well, if you really need this...
# install(FILES saplayer-old DESTINATION bin)
