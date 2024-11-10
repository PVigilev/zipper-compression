cmake_minimum_required(VERSION 3.5.0)

project(zipper-compression-tests)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# enable_testing()

add_executable(zipper-compression-tests
	deflate_decoder_tests.cpp
)

target_link_libraries(zipper-compression-tests
   zipper-compression-library
   GTest::gtest_main
)

# include_directories(../src
# )

include(GoogleTest)

gtest_discover_tests(zipper-compression-tests)