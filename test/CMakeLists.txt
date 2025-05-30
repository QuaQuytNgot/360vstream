cmake_minimum_required(VERSION 3.21)

project(vstreamTests LANGUAGES C)

include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(vstream REQUIRED)
  enable_testing()
endif()

# ---- Tests ----

add_executable(vstream_test source/vstream_test.c)
target_link_libraries(vstream_test PRIVATE vstream::vstream)
target_compile_features(vstream_test PRIVATE c_std_17)

add_test(NAME vstream_test COMMAND vstream_test)

# ---- End-of-file commands ----

add_folders(Test)
