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

add_executable(local-conn)
target_sources(local-conn PRIVATE  "local-connectivity.cpp")
target_link_libraries(local-conn PRIVATE skel ${ITK_LIBRARIES})

add_executable(outside-ms)
target_sources(outside-ms PRIVATE  "medial-surface-outside.cpp")
target_link_libraries(outside-ms PRIVATE skel ${ITK_LIBRARIES})

add_executable(count_loops)
target_sources(count_loops PRIVATE
    "loops/count_loops.cpp"
    "loops/graph.cpp"
    "loops/loops.cpp"
)

target_include_directories(count_loops PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/loops
                           )
target_link_libraries(count_loops PRIVATE skel ${ITK_LIBRARIES})
