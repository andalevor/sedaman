enable_testing()

add_executable(ascii_ebcdic_conversion ascii_ebcdic_conversion.cpp)
add_test(ascii_ebcdic_conversion_test ascii_ebcdic_conversion)
target_link_libraries(ascii_ebcdic_conversion sedaman)

add_executable(print_text_header print_text_header.cpp)
add_test(print_text_header_test print_text_header ${PROJECT_SOURCE_DIR}/samples/ibm.sgy)
target_link_libraries(print_text_header sedaman)

add_executable(print_bin_header print_bin_header.cpp)
add_test(print_bin_header_test print_bin_header ${PROJECT_SOURCE_DIR}/samples/ibm.sgy)
target_link_libraries(print_bin_header sedaman)

add_executable(no_such_file no_such_file.cpp)
add_test(no_such_file_test no_such_file ${PROJECT_SOURCE_DIR}/samples/no_such_file)
target_link_libraries(no_such_file sedaman)

add_executable(check_bin_header check_bin_header.cpp)
add_test(check_bin_header_test check_bin_header ${PROJECT_SOURCE_DIR}/samples/ibm.sgy)
target_link_libraries(check_bin_header sedaman)

add_executable(read_all_headers read_all_headers.cpp)
add_test(read_all_headers_test read_all_headers ${PROJECT_SOURCE_DIR}/samples/ibm.sgy)
target_link_libraries(read_all_headers sedaman)

add_executable(count_all_traces count_all_traces.cpp)
add_test(count_all_traces_test count_all_traces ${PROJECT_SOURCE_DIR}/samples/ibm.sgy)
target_link_libraries(count_all_traces sedaman)

add_executable(read_trace_samples read_trace_samples.cpp)
add_test(read_trace_samples_test read_trace_samples ${PROJECT_SOURCE_DIR}/samples/ibm.sgy)
target_link_libraries(read_trace_samples sedaman)

add_executable(read_and_write read_and_write.cpp)
add_test(read_and_write_test read_and_write ${PROJECT_SOURCE_DIR}/samples/ibm.sgy test.sgy)
target_link_libraries(read_and_write sedaman)
