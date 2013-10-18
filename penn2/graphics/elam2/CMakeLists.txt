project(elam2)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_CXX_FLAGS "-DNDEBUG -O3 -g -std=c++0x")
#aux_source_directory(. SRC_LIST)
SET(SRC_LIST main.cpp hud_Element.cpp hud_Element_circle.cpp textBox utils.cpp)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} zmq osg osgDB osgUtil osgGA osgViewer osgText osgGA osgUtil osgParticle osgAnimation OpenThreads)

