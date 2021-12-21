cmake_minimum_required(VERSION 3.21)
project(checkers)

set(CMAKE_CXX_STANDARD 17)

add_executable(checkers main.cpp board.cpp board.h)
