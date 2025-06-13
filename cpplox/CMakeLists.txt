cmake_minimum_required(VERSION 3.5.0)
project(cpplox VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -Werror -Wpedantic")

add_executable(cpplox src/main.cpp src/compiler.cpp src/debug.cpp src/file.cpp src/chunk.cpp src/interned.cpp src/memory.cpp src/parser.cpp src/scanner.cpp src/string.cpp src/vm.cpp)
target_link_libraries(cpplox PRIVATE "-lstdc++exp")


