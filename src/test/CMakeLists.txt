file(GLOB ips_test_srcs "*.cpp")

enable_testing()

add_executable(test.or ${ips_test_srcs})
target_link_libraries(test.or mathorlib pthread rt)
