cmake_minimum_required(VERSION 2.8)

project(helios)

#set(CMAKE_CXX_FLAGS "-O3 -g -Wall -DNDEBUG")
set(CMAKE_CXX_FLAGS "-O3 -g -Wall")
set(CMAKE_CXX_LINK_FLAGS "-lrt")

add_executable(helios main.cpp pelops.cpp blkdiag.cpp ComputerEEGFilterCursorOnly.cpp ComputerEEGFilterRSE.cpp
 constructRSEMatrices.cpp generateChannelParameters.cpp nextTimeStep.cpp parameters.cpp randomStartLocation.cpp
 repslices.cpp tweakbargui.cpp)

target_link_libraries(helios armadillo usb-1.0 cairo OpenThreads osg osgDB osgUtil osgGA osgViewer osgText osgGA
 osgUtil osgParticle AntTweakBar opencv_core opencv_features2d opencv_calib3d opencv_imgproc opencv_video opencv_videostab opencv_highgui tbb)
