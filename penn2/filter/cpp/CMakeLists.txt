project(filter)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "-DNDEBUG -O3")
set(SRC_LIST main.cpp filter_class.cpp naive_filter.cpp reachstateequation.cpp jointrse_filter.cpp)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries (${PROJECT_NAME} zmq armadillo)

