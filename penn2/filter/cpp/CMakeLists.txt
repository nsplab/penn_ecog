project(filter)
cmake_minimum_required(VERSION 2.8)
#set(CMAKE_CXX_FLAGS "-DNDEBUG -O3")
set(CMAKE_CXX_FLAGS "-O3 -Wall -g -std=gnu++11")
set(SRC_LIST main.cpp filter_class.cpp naive_filter.cpp reachstateequation.cpp jointrse_filter.cpp tests.cpp matrix.cpp)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries (${PROJECT_NAME} zmq armadillo itpp fltk)

