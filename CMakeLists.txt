cmake_minimum_required (VERSION 3.21.3)
project(prefix-doubling)

set(CMAKE_C_FLAGS "-Wall -Wextra -O3")
add_executable(pd main.c shared.c pd-1.c pd-2.c)
