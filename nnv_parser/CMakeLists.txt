cmake_minimum_required(VERSION 3.19.7)

project(nnv)

add_executable(nnv ./src/main.cpp)

target_include_directories(nnv PRIVATE ./include)
