add_executable(process_test process_test.cc)
target_link_libraries(process_test gtest backend)

add_executable(server_test server_test.cc)
target_link_libraries(server_test gtest backend)