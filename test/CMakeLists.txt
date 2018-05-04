find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories(${PROJECT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_TEST_DYN_LINK)

set(TEST_PATH "${CMAKE_CURRENT_BINARY_DIR}/")

configure_file(buildpaths.py.in ${CMAKE_CURRENT_BINARY_DIR}/buildpaths.py)
configure_file(buildpaths.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/test_buildpaths.cpp)
add_library(libtestdgen STATIC ${CMAKE_CURRENT_BINARY_DIR}/test_buildpaths.cpp)

# copy files
configure_file(words1.txt ${TEST_PATH}/words1.txt COPYONLY)
configure_file(json_conf1.txt ${TEST_PATH}/json_conf1.txt)
configure_file(json_conf2.txt ${TEST_PATH}/json_conf2.txt)
configure_file(json_conf3.txt ${TEST_PATH}/json_conf3.txt)
configure_file(json_conf4.txt ${TEST_PATH}/json_conf4.txt)

configure_file(cards.py ${TEST_PATH}/cards.py COPYONLY)

configure_file(bench_conf1.txt ${TEST_PATH}/bench_conf1.txt)

add_executable(str str_format.cpp)
target_link_libraries(str libdgen libtestdgen ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(NAME StrFormat COMMAND str)

add_executable(dict dict_test.cpp)
target_link_libraries(dict libdgen libtestdgen ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(NAME Dict COMMAND dict)
# add_dependencies(dict ${TEST_PATH}/words1.txt)

add_executable(config config_test.cpp)
target_link_libraries(config libdgen libtestdgen ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(NAME ConfigTest COMMAND config)

add_executable(csv csv_test.cpp)
target_link_libraries(csv libdgen libtestdgen ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(NAME CSV COMMAND csv)

find_package(PythonInterp 2.7 REQUIRED)
add_test(NAME Card COMMAND ${PYTHON_EXECUTABLE} ${TEST_PATH}/cards.py)

# benchmark
configure_file(bench.py ${TEST_PATH}/bench.py COPYONLY)