add_library(drone_model STATIC src/Drone.cpp src/Target.cpp)

target_include_directories(drone_model PUBLIC include)
target_compile_options(drone_model PRIVATE -Wall -Wextra -pedantic)
