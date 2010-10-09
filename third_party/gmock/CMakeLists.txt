include_directories(. include)

add_library(gmock
  src/gmock-all.cc)
set_target_properties(gmock
  PROPERTIES
  COMPILE_FLAGS "-DPATH_MAX=4096")
