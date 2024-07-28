cmake_minimum_required(VERSION 3.10)
project(CyclomaticComplexityAnalysis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions(${LLVM_DEFINITIONS})

add_library(CyclomaticComplexity SHARED
    src/CyclomaticComplexity.cpp
)

target_link_libraries(CyclomaticComplexity
    PRIVATE
    clang-cpp
    LLVM
)