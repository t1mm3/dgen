cmake_minimum_required(VERSION 3.0)

project(Dgen VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "No build type specified. Assuming Release build.")
	set(CMAKE_BUILD_TYPE "RELEASE" CACHE "Build type" STRING FORCE)
endif()

ADD_DEFINITIONS(
  -g
  -Wall
  -pedantic
  -pedantic-errors
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ") # -fsanitize=address
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -O3 -ftree-vectorize -ftree-slp-vectorize -funroll-all-loops -fpeel-loops -fomit-frame-pointer")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/concurrentqueue)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/variant-1.1.5/include)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")

enable_testing()
add_subdirectory(src)
add_subdirectory(comparisons)