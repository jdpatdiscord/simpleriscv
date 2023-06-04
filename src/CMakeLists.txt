cmake_minimum_required(VERSION 3.15)
project(RISCVContainerProject)

add_library(RISCVContainer STATIC riscv_vm.cpp)
target_include_directories(RISCVContainer PUBLIC include/)
target_compile_options(RISCVContainer PUBLIC -O2 -std=c++20 -Wall -Wextra)