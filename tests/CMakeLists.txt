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

add_executable(check_reading_samples_ibm check_reading_samples_ibm.cpp)
add_test(check_reading_samples_ibm_test check_reading_samples_ibm ${PROJECT_SOURCE_DIR}/samples/ibm.sgy)
target_link_libraries(check_reading_samples_ibm sedaman)

add_executable(check_reading_samples_ieee_single check_reading_samples_ieee_single.cpp)
add_test(check_reading_samples_ieee_single_test check_reading_samples_ieee_single ${PROJECT_SOURCE_DIR}/samples/ieee_single.sgy)
target_link_libraries(check_reading_samples_ieee_single sedaman)

add_executable(check_reading_samples_4I check_reading_samples_4I.cpp)
add_test(check_reading_samples_4I_test check_reading_samples_4I ${PROJECT_SOURCE_DIR}/samples/4I.sgy)
target_link_libraries(check_reading_samples_4I sedaman)

add_executable(check_reading_samples_2I check_reading_samples_2I.cpp)
add_test(check_reading_samples_2I_test check_reading_samples_2I ${PROJECT_SOURCE_DIR}/samples/2I.sgy)
target_link_libraries(check_reading_samples_2I sedaman)

add_executable(check_reading_samples_1I check_reading_samples_1I.cpp)
add_test(check_reading_samples_1I_test check_reading_samples_1I ${PROJECT_SOURCE_DIR}/samples/1I.sgy)
target_link_libraries(check_reading_samples_1I sedaman)

add_executable(verify_writing verify_writing.cpp)
add_test(verify_writing_ibm_test verify_writing ${PROJECT_SOURCE_DIR}/samples/ibm.sgy test_ibm.sgy)
add_test(verify_writing_ieee_single_test verify_writing ${PROJECT_SOURCE_DIR}/samples/ieee_single.sgy test_ieee_single.sgy)
add_test(verify_writing_4I_test verify_writing ${PROJECT_SOURCE_DIR}/samples/4I.sgy test_4I.sgy)
add_test(verify_writing_2I_test verify_writing ${PROJECT_SOURCE_DIR}/samples/2I.sgy test_2I.sgy)
add_test(verify_writing_1I_test verify_writing ${PROJECT_SOURCE_DIR}/samples/1I.sgy test_1I.sgy)
target_link_libraries(verify_writing sedaman)
