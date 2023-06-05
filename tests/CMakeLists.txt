cmake_minimum_required(VERSION 3.15)
project(RISCVTests)

add_subdirectory(../src ./bin/)

add_executable(TestRunner testrunner.cpp)
target_compile_options(TestRunner PUBLIC -std=c++20 -Wall -Wextra -O2)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY testbin/)

add_executable(AddTest src/add.cpp)
target_compile_options(AddTest PUBLIC -std=c++20 -Wall -Wextra -O2)
target_link_directories(AddTest PUBLIC ../build)
target_link_libraries(AddTest RISCVContainer)
target_include_directories(AddTest PUBLIC ../include)