cmake_minimum_required(VERSION 3.31.6)

project(ringcore)

set(CMAKE_CXX_STANDARD 20)

include(CTest)
enable_testing()

add_subdirectory(tests)
add_subdirectory(benchmarks)

