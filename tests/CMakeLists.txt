cmake_minimum_required(VERSION 3.15)
project(RISCVTests)

add_executable(TestRunner testrunner.cpp)

add_executable(BranchTest src/branch.cpp)
target_compile_options(BranchTest PUBLIC -std=c++20 -Wall -Wextra -O2)
target_link_directories(BranchTest PUBLIC ../build)
target_link_libraries(BranchTest -lRISCVContainer)
target_include_directories(BranchTest PUBLIC ../include)