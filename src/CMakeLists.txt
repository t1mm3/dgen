## Required software
find_package(Threads REQUIRED)

find_package(Boost 1.40 COMPONENTS program_options iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

## Generate tables
find_package(PythonInterp 2.7 REQUIRED)

add_custom_command(
  COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/gen_tables.py" > "${CMAKE_CURRENT_BINARY_DIR}/gen_tables.hpp" 
  DEPENDS gen_tables.py
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gen_tables.hpp
  COMMENT "Generating tables"
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/gen_tables.hpp GENERATED)


## Build information
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file(build.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/build.cpp)

## Sources and headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(libdgen STATIC outputs.cpp pool.cpp conf.cpp generator.cpp utils.cpp output_queue.cpp buffer.cpp primitives.cpp
	${CMAKE_CURRENT_BINARY_DIR}/gen_tables.hpp
	dict.cpp dict_impl1.cpp dict_impl2.cpp dict_impl3.cpp dict_impl4.cpp dict_impl5.cpp dict_impl6.cpp dict_impl7.cpp
	${CMAKE_CURRENT_BINARY_DIR}/build.cpp)
target_link_libraries(libdgen ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

add_executable(dgen gen.cpp)
target_link_libraries(dgen libdgen)

## Testing
enable_testing()
add_subdirectory(test)
