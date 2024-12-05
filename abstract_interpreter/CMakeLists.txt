cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0135 NEW)

project(absint
    HOMEPAGE_URL "https://github.com/ptal/lattice-theory-parallel-programming-uni.lu"
    LANGUAGES CXX)

# We use cpp-peglib to parse C programs.
include(FetchContent)
FetchContent_Declare(
    cpp_peglib
    GIT_REPOSITORY "https://github.com/ptal/cpp-peglib.git"
    GIT_TAG        245b59446d424b499b316ee77ec59de18443de23
)
FetchContent_MakeAvailable(cpp_peglib)

add_executable(absint src/main.cpp)
target_include_directories(absint PRIVATE include)
target_compile_features(absint PRIVATE cxx_std_17)
target_link_libraries(absint cpp_peglib)
