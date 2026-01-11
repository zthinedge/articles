cmake_minimum_required(VERSION 3.14)
project(MemoryPoolProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(test_pool
    src/MemoryPool.cpp
    test/main.cpp
)