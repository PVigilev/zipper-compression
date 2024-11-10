cmake_minimum_required(VERSION 3.22.0)

project(zipper-compression)


set(CMAKE_CXX_STANDARD 23)

include_directories(include)

add_subdirectory(src)

add_subdirectory(cli)

enable_testing()
add_subdirectory(tests)

