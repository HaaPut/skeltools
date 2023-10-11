cmake_minimum_required(VERSION 3.13)
message(STATUS "Building Examples")

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3  -Wall -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)


file(COPY ${PROJECT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(homotopic-centerline)
target_sources(homotopic-centerline PRIVATE  "homotopic-centerlines.cpp")
target_link_libraries(homotopic-centerline PRIVATE skel ${ITK_LIBRARIES})

add_executable(map-thickness)
target_sources(map-thickness PRIVATE  "map-medial-thickness-to-boundary.cpp")
target_link_libraries(map-thickness PRIVATE skel ${ITK_LIBRARIES})

