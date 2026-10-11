cmake_minimum_required(VERSION 3.15)

project("MaxScript")

set(TESTING ON)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


add_executable("MaxScript" "source/test.cpp" "source/runtime.cpp" "source/systemfunc.cpp" "source/bytecode.cpp")

set_property(TARGET MaxScript PROPERTY CXX_STANDARD 23)

target_include_directories(MaxScript PUBLIC include)