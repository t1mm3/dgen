cmake_minimum_required(VERSION 2.8)

project(Dgen)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


ADD_DEFINITIONS(
  -g
  -Wall
  -pedantic
  -pedantic-errors
  -march=native -O3
)

## Required software
find_package(Threads REQUIRED)

find_package(Boost 1.40 COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

## Sources and headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dgen gen.cpp outputs.cpp pool.cpp conf.cpp generator.cpp)
target_link_libraries(dgen ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})