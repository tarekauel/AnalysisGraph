cmake_minimum_required(VERSION 2.8.4)
project(AnalysisGraph)

MESSAGE(STATUS "Test")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS regex)

find_package(TBB REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${TBB_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp graph.h graph.cpp vertex.h vertex.cpp relationship.h relationship.cpp spreading_activation.cpp)
add_executable(AnalysisGraphTool ${SOURCE_FILES})

target_link_libraries(AnalysisGraphTool ${Boost_LIBRARIES})
target_link_libraries(AnalysisGraphTool ${TBB_LIBRARIES})