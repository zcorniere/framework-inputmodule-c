add_executable(Test test.cpp)

target_link_libraries(Test PRIVATE framework_inputmodule_static)
set_property(TARGET Test PROPERTY CXX_STANDARD 20)
