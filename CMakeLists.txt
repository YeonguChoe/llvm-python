cmake_minimum_required(VERSION 4.0)
project(llvm-python)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main main.cpp lexer.cpp parser.cpp)