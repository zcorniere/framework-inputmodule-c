cmake_minimum_required(VERSION 3.20)
project(framework_inputmodule)

add_library(${PROJECT_NAME} OBJECT src/InputModule.cxx)
target_include_directories(${PROJECT_NAME} PUBLIC src)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(${PROJECT_NAME}_shared SHARED $<TARGET_OBJECTS:${PROJECT_NAME}>)
target_include_directories(${PROJECT_NAME}_shared PUBLIC src)
add_library(${PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}>)
target_include_directories(${PROJECT_NAME}_static PUBLIC src)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    message(STATUS "Documentation using Doxygen enabled")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
    set(DOXYGEN_WARN_LOGFILE ${CMAKE_BINARY_DIR}/doxygen-warnings.log)
    set(DOXYGEN_PREDEFINED DOXYGEN_SHOULD_SKIP_THIS)
    doxygen_add_docs(
        doc
        src/
        README.md
    )
else()
    message(STATUS"Doxygen needs to be installed to build the documentation")
endif()

add_subdirectory(tests)


# Option to force colored output of the compiler
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if(${FORCE_COLORED_OUTPUT})
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
endif()
