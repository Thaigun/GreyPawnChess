cmake_minimum_required(VERSION 3.14)
project(greypawnchess_test)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG origin/main
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# This setup seems and feels wrong and there is probably a better way to do it.
# It was hacked together by a CMake noob based on derstood copy-pasta snippets.
# Keep them in alphabetical order!
add_executable(
    EngineTest
    # Test files
    BoardTest.cpp
    MonteCarloNodeTest.cpp
    MoveTest.cpp
    PieceTest.cpp
    # Engine files
    ../src/Board.cpp
    ../src/BoardFuncs.cpp
    ../src/GameState.cpp
    ../src/GreyPawnChess.cpp
    ../src/MonteCarloNode.cpp
    ../src/Move.cpp
    ../src/PGNParsing.cpp
    ../src/Random.cpp
    ../src/TimeManagement.cpp
)
target_link_libraries(
    EngineTest
    gtest_main
)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include(GoogleTest)
gtest_discover_tests(EngineTest)