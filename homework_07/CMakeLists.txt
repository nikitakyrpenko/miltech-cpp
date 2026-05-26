cmake_minimum_required(VERSION 3.20)

project(drone_target_calc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(drone_target_cli
  src/main.cpp
  src/models/Ammo.cpp
  src/models/Drone.cpp
  src/models/Target.cpp
  src/service/AnalyticalSolver.cpp
)
target_include_directories(drone_target_cli PUBLIC include)
target_compile_options(drone_target_cli PRIVATE -Wall -Wextra -pedantic)

if(NOT CMAKE_CROSSCOMPILING)
  include(FetchContent)
  include(GoogleTest)
  FetchContent_Declare(
    googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
               DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(googletest)

  add_executable(drone_target_calc_tests
    tests/AnalyticalSolver.cpp
    src/models/Ammo.cpp
    src/models/Drone.cpp
    src/models/Target.cpp
    src/service/AnalyticalSolver.cpp
  )
  target_include_directories(drone_target_calc_tests PRIVATE include)
  target_link_libraries(drone_target_calc_tests PUBLIC GTest::gtest_main)
  gtest_discover_tests(drone_target_calc_tests DISCOVERY_MODE PRE_TEST PROPERTIES LABELS "homework_07")

endif()
