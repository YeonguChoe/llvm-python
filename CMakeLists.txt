cmake_minimum_required(VERSION 4.0)
project(llvm-python)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(llvm-python lexer.cpp main.cpp)

find_package(zstd REQUIRED)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs support)
target_link_libraries(llvm-python PRIVATE ${llvm_libs})