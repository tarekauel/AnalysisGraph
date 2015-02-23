set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/")
find_package(Boost COMPONENTS regex)

find_package(TBB REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${TBB_INCLUDE_DIRS})

set(SOURCE_FILES_GRAPH_TOOL graph.h graph.cpp vertex.h vertex.cpp relationship.h relationship.cpp impuls.h spreading_activation.cpp shortest_path.h shortest_path.cpp distance_algorithm.h distance_algorithm.cpp)

add_library(analysisGraphLib ${SOURCE_FILES_GRAPH_TOOL})

target_link_libraries(analysisGraphLib ${Boost_LIBRARIES})
target_link_libraries(analysisGraphLib ${TBB_LIBRARIES})