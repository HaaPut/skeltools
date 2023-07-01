cmake_minimum_required(VERSION 3.13)
message(STATUS "Building Examples")

set(ITK_DIR $ENV{HOME}/opt/itk/lib/cmake/ITK-5.3)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3  -Wall -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)


file(COPY ${PROJECT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(homotopic-centerline)
target_sources(homotopic-centerline PRIVATE  "homotopic-centerlines.cpp")
target_link_libraries(homotopic-centerline PRIVATE skel ${ITK_LIBRARIES})

