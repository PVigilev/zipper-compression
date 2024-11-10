cmake_minimum_required(VERSION 3.5.0)

project(zipper-compression-library)

add_library(${PROJECT_NAME}
    logger.cpp
    deflate/decoder.cpp
)

