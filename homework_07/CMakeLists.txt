cmake_minimum_required(VERSION 3.20)

project(drone_target_calc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(drone_target_cli src/main.cpp src/Drone.cpp src/Target.cpp)
target_include_directories(drone_target_cli PUBLIC include)
target_compile_options(drone_target_cli PRIVATE -Wall -Wextra -pedantic)
