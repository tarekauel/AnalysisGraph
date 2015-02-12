cmake_minimum_required(VERSION 2.8.4)
project(AnalysisGraph)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp graph.h graph.cpp vertex.h vertex.cpp relationship.h relationship.cpp graph_algorithm.cpp)
add_executable(AnalysisGraph ${SOURCE_FILES})