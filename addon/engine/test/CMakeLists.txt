cmake_minimum_required(VERSION 3.14)
project(greypawnchess_test)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    MoveTest
    MoveTest.cpp
)
target_link_libraries(
    MoveTest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(MoveTest)