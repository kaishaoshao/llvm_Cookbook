find_package(GTest REQUIRED)

add_library(test_common INTERFACE)
target_link_libraries(test_common INTERFACE toy_headers GTest::gtest)

# Chapter-specific tests live under test/chN and should mirror the
# corresponding mlir-chN target naming.
add_subdirectory(ch1)
add_subdirectory(ch2)
