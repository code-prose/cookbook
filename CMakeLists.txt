cmake_minimum_required(VERSION 3.20)
project(cookbook)

set(CMAKE_CXX_STANDARD 23)


add_executable(cookbook
    src/main.cpp
    src/order.cpp
)

target_include_directories(cookbook PRIVATE include)
