cmake_minimum_required(VERSION 3.7)
set(PROJECT "gbflasher")
project(${PROJECT})

set(base_DIR ${CMAKE_CURRENT_LIST_DIR})

set(src_DIR ${base_DIR}/src)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${src_DIR}/cmake")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-visibility")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

find_package(LIBHIDAPIRAW REQUIRED)

include_directories(
    include
    ${LIBHIDAPIRAW_INCLUDE_DIR}
)

set(HEADERS
    include/appinfo.h
    include/deviceinfo.h
    include/ext/bufferstream.h
    include/flasher.h
    include/flashfile.h
    include/hid.h
    include/main.h
    include/memoryinfo.h
    include/timer.h
    include/utils/hex.h
)

set(SOURCES
    src/appinfo.cpp
    src/deviceinfo.cpp
    src/ext/bufferstream.cpp
    src/flasher.cpp
    src/flashfile.cpp
    src/hid.cpp
    src/main.cpp
    src/memoryinfo.cpp
    src/timer.cpp
    src/utils/hex.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_executable(${PROJECT} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT} ${LIBHIDAPIRAW_LIBRARIES})
