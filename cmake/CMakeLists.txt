cmake_minimum_required(VERSION 3.15)

project("MaxScript")

set(TESTING ON)




add_executable("MaxScript" "source/test.cpp" "source/runtime.cpp" "source/systemfunc.cpp" "source/bytecode.cpp")

set_property(TARGET MaxScript PROPERTY CXX_STANDARD 23)

target_include_directories(MaxScript PUBLIC include)