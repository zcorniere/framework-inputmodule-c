cmake_minimum_required(VERSION 3.20)
project(framework_inputmodule)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE src/)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

if(WIN32)
    # Don't know what Windows needs yet
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} INTERFACE udev)
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        message(STATUS "Documentation using Doxygen enabled")
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
        set(DOXYGEN_WARN_LOGFILE ${CMAKE_BINARY_DIR}/doxygen-warnings.log)
        set(DOXYGEN_PREDEFINED DOXYGEN_SHOULD_SKIP_THIS)
        doxygen_add_docs(doc src/ README.md)
    else()
        message(STATUS"Doxygen needs to be installed to build the documentation")
    endif()

    add_subdirectory(tests)

endif(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
