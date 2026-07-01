cmake_minimum_required(VERSION 3.20)

project(drone_target_calc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# tests temporarily disabled; re-enable with -DBUILD_TESTS=ON
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
    include(FetchContent)
    include(GoogleTest)
    FetchContent_Declare(
        googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
                       DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
endif()

add_subdirectory(drone_model)
add_subdirectory(drone_utils)
add_subdirectory(drone_state)
add_subdirectory(drone_service)
add_subdirectory(drone_transport)

add_executable(drone_target_cli src/Main.cpp)
target_link_libraries(drone_target_cli PRIVATE drone_model drone_state drone_service drone_transport)
target_include_directories(drone_target_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external)
target_compile_options(drone_target_cli PRIVATE -Wall -Wextra -pedantic)
