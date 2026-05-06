cmake_minimum_required(VERSION 3.20)

project(drone_balistic CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(drone_balistic src/main.cpp)

file(COPY data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(drone_balistic PRIVATE include)

target_compile_options(drone_balistic PRIVATE -Wall -Wextra -pedantic )