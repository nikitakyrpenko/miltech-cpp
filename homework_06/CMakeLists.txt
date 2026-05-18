cmake_minimum_required(VERSION 3.20)

project(drone_ballistic_cli CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(drone_ballistic_calc src/ballistic.cpp)
target_include_directories(drone_ballistic_calc PUBLIC include)

add_executable(drone_ballistic_cli src/main.cpp)
target_link_libraries(drone_ballistic_cli PRIVATE drone_ballistic_calc)
target_compile_options(drone_ballistic_cli PRIVATE -Wall -Wextra -pedantic)

if(NOT CMAKE_CROSSCOMPILING)
    include(FetchContent)
    include(GoogleTest)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(drone_ballistic_tests tests/ballistic_tests.cpp)
    target_link_libraries(drone_ballistic_tests PUBLIC drone_ballistic_calc GTest::gtest_main)
    gtest_discover_tests(drone_ballistic_tests DISCOVERY_MODE PRE_TEST)

endif()