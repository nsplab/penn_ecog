project(fixbrokenfiles)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_CXX_FLAGS "-O0 -g -std=c++0x")
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

