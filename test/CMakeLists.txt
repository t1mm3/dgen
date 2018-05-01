find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories(${PROJECT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_TEST_DYN_LINK)


add_executable(str str_format.cpp)
target_link_libraries(str libdgen ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(NAME StrFormat COMMAND str)

add_executable(csv csv_test.cpp)
target_link_libraries(csv libdgen ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(NAME CSV COMMAND csv)