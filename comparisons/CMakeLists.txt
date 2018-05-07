configure_file(runner.py ${CMAKE_CURRENT_BINARY_DIR}/runner.py)

configure_file(conf1.json ${CMAKE_CURRENT_BINARY_DIR}/conf1.json COPYONLY)

add_executable(csv_naive_printf csv_naive_printf)
add_executable(csv_naive_cppstream csv_naive_cppstream)
