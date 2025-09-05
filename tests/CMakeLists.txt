add_executable(test_c_x test_c_x.c ${CMAKE_SOURCE_DIR}/src/c_x.c)
target_include_directories(test_c_x PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME test_c_x COMMAND test_c_x)

