configure_file(runner.py ${CMAKE_CURRENT_BINARY_DIR}/runner.py)
configure_file(plot.py ${CMAKE_CURRENT_BINARY_DIR}/plot.py)

configure_file(conf1.json ${CMAKE_CURRENT_BINARY_DIR}/conf1.json COPYONLY)

add_executable(csv_naive_printf csv_naive_printf.cpp)
add_executable(csv_naive_cppstream csv_naive_cppstream.cpp)
add_executable(csv_hardcode_lut csv_hardcode_lut.cpp)