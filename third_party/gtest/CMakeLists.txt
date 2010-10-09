include_directories(.)

add_library(gtest
  src/gtest-all.cc)
set_target_properties(gtest
  PROPERTIES
  COMPILE_FLAGS "-DPATH_MAX=4096")

add_library(gtest_main src/gtest_main.cc)
target_link_libraries(gtest_main gtest)
