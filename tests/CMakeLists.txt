enable_testing()

add_executable(ascii_ebcdic_conversion ascii_ebcdic_conversion.cpp)
add_test(ascii_ebcdic_conversion_test ascii_ebcdic_conversion)
target_link_libraries(ascii_ebcdic_conversion sedaman)
add_executable(bin_bcd_conversion bin_bcd_conversion.cpp)
add_test(bin_bcd_conversion_test bin_bcd_conversion)
target_link_libraries(bin_bcd_conversion sedaman)

add_subdirectory(segy)
add_subdirectory(segd)