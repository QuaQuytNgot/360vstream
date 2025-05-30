cmake_minimum_required(VERSION 3.22.0)

project(my_program C CXX)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SHARED_SOURCE
    ${PROJECT_SOURCE_DIR}/src/*.c)

add_executable(my_program ${SHARED_SOURCE} my_program/main.c)

target_link_libraries(my_program /home/ohmypc/curl/build/lib/libcurl.so)
target_link_libraries(my_program m)
