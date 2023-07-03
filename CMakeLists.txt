cmake_minimum_required(VERSION 3.13)
project(skeltools VERSION 1.0
        DESCRIPTION "Skeletonization Tools"
        LANGUAGES CXX)
message(STATUS "Found CMAKE-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}")
message(STATUS "----------------Running CMake---------------------------------")
cmake_host_system_information(RESULT HOSTNAME QUERY HOSTNAME)
message(STATUS "Invoking CMAKE on : ${HOSTNAME} running: ${CMAKE_HOST_SYSTEM}")

set(ITK_DIR $ENV{HOME}/opt/itk/lib/cmake/ITK-5.3)
message(STATUS "ITK Directory ${ITK_DIR}")

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})


set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Set a default build type if none was specified
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
    STRING "Choose the type of build." FORCE)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)

set(DEBUG_CXX_FLAGS "-ggdb;-O0;-Wall;-m64;-fPIC;-std=c++17")
set(RELEASE_CXX_FLAGS "-O3;-Wall;-m64;-fPIC;-std=c++17")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)



# Sources
set(SOURCES
        src/itkCommandLineArgumentParser.cxx
        src/topology.cpp
		src/experiment.cpp
		src/homotopic.cpp
		src/medial.cpp
        )

set(HEADERS
        include/itkCommandLineArgumentParser.h
        include/topology.h
        include/itkHomotopicThinningImageFilter.h
		include/experiment.h
		include/homotopic.h
		include/util.h
		include/flux.h
		include/medial.h
		include/itkSpokeFieldToAverageOutwardFluxImageFilter.h
        )

add_library(skel SHARED)

target_compile_options(skel PUBLIC "$<$<CONFIG:Debug>:${DEBUG_CXX_FLAGS}>")
target_compile_options(skel PUBLIC "$<$<CONFIG:Release>:${RELEASE_CXX_FLAGS}>")

target_compile_definitions(skel PUBLIC "$<$<CONFIG:Debug>:DEBUG_BUILD>")

target_include_directories(skel PUBLIC include)

target_sources(skel PRIVATE ${SOURCES}
		PUBLIC ${HEADERS})

set_target_properties(skel PROPERTIES
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        )

add_executable(skeltool main.cpp)

target_link_libraries(skeltool skel ${ITK_LIBRARIES})

# Tests
#enable_testing()
#add_subdirectory(tests)

add_subdirectory("examples")
