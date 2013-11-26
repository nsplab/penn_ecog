project(canal)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "-Wall -O3 -std=gnu++11 -fopenmp")
SET(SRC_LIST main.cpp)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} zmq fftw3f_omp fftw3f)

