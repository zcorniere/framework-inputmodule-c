cmake_minimum_required(VERSION 3.29)
project(framework_inputmodule)

add_library(${PROJECT_NAME} OBJECT src/inputmodule.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(framework_inputmodule_shared SHARED $<TARGET_OBJECTS:${PROJECT_NAME}>)
add_library(framework_inputmodule_static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}>)
