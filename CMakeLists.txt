cmake_minimum_required(VERSION 3.0)

project(Dgen VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

ADD_DEFINITIONS(
  -g
  -Wall
  -pedantic
  -pedantic-errors
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -O3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/variant/include)

## Required software
find_package(Threads REQUIRED)

find_package(Boost 1.40 COMPONENTS program_options iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

## Build information
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file(build.cpp.in ${PROJECT_BINARY_DIR}/build.cpp)

## Sources and headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(libdgen STATIC outputs.cpp pool.cpp conf.cpp generator.cpp utils.cpp dict.cpp ${PROJECT_BINARY_DIR}/build.cpp)
target_link_libraries(libdgen ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

add_executable(dgen gen.cpp)
target_link_libraries(dgen libdgen)

## Testing
enable_testing()
add_subdirectory(test)