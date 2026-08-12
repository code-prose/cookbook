cmake_minimum_required(VERSION 3.20)
project(cookbook)

set(CMAKE_CXX_STANDARD 23)


add_executable(cookbook
    src/main.cpp
    src/order.cpp
    src/event_loop.cpp
    src/data_feed.cpp
    src/strategy.cpp
)

target_include_directories(cookbook PRIVATE include)
target_compile_options(cookbook PRIVATE -mavx512f -mavx512bw -mbmi)

add_executable(microbench
    src/microbench.cpp
    src/data_feed.cpp
)

target_include_directories(microbench PRIVATE include)
target_compile_options(microbench PRIVATE -O2 -g -fno-omit-frame-pointer -mavx512f -mavx512bw -mbmi)

add_executable(test
    src/test.cpp
)

target_include_directories(test PRIVATE include)
target_compile_options(test PRIVATE -O2 -g -fno-omit-frame-pointer -mavx512f -mavx512bw -mbmi)
