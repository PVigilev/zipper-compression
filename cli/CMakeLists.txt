cmake_minimum_required(VERSION 3.5.0)

project(zipper-compression-cli)

add_executable(${PROJECT_NAME}
    main.cpp
)

# target_link_libraries(${PROJECT_NAME}
#     zipper-compression-library
# )