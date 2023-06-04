# SimpleRISCV
A simple RISC-V implementation that aims to be source-as-documentation for the architecture and its extensions

# Building & testing 
```
git clone https://github.com/jdpatdiscord/simpleriscv
cd simpleriscv/tests/
cmake -S . -B build
cmake --build build
cd build
# Windows:
./TestRunner.exe
# others:
./TestRunner
```

# Contributor guidelines
* Any pull request opened must leave the master branch in a compiling & working state. 
* Must maintain ISO C++20. If any speed benefits are are possible with non-ISO C++20, it must be under preprocessor check.
* All C++ code must be clear and concise, accessible to a C programmer. This shouldn't be a demonstration of C++ syntax.
* Any instruction added henceforth should have a test case. If you see an instruction without a test case, you may add it.
* Make commit messages clear and please explain all of what you are doing. Keep extraneous commits to a minimum.
* If what you are working on pertains to RISC-V, and may be complicated, please write concise comments about it.
* If your commit is relevant to information displayed on the README, please update it in your pull request.

# Current goals
* Reach full RV32I compatibility
* Write a full test suite for instructions, then branch out testing for different OS/compiler platforms as well.
