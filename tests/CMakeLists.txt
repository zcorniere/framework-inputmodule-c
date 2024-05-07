add_executable(Test test.cpp)

target_link_libraries(Test PUBLIC framework_inputmodule)
set_property(TARGET Test PROPERTY CXX_STANDARD 20)
