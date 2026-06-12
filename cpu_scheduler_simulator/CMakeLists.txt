cmake_minimum_required(VERSION 3.12)
project(cpu_scheduler_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cpu_scheduler_simulator
    src/main.cpp
    src/scheduler.cpp
)

target_include_directories(cpu_scheduler_simulator PRIVATE src)

target_compile_options(cpu_scheduler_simulator PRIVATE
    -Wall -Wextra -Wpedantic
)
