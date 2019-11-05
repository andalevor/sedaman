enable_testing()

add_executable(trace_move trace_move.cpp)
add_test(trace_move_test trace_move)
target_link_libraries(trace_move sedaman)

add_executable(trace_copy trace_copy.cpp)
add_test(trace_copy_test trace_copy)
target_link_libraries(trace_copy sedaman)


