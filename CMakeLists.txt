cmake_minimum_required(VERSION 3.0)
project(wenity)

set(CMAKE_C_FLAGS "-static ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-static ${CMAKE_CXX_FLAGS}")
add_definitions(-std=c++0x)
add_executable(wenity main.cpp)
target_link_libraries(wenity shell32 comdlg32)
