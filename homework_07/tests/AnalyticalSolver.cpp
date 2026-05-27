#include "service/AnalyticalSolver.hpp"
#include "models/Drone.hpp"
#include "models/DroneBuilder.hpp"
#include "models/Ammo.hpp"

#include <gtest/gtest.h>
#include <iostream>

TEST(test, test)
{
  auto as = AnalyticalSolver{};

  auto drone = Drone::builder()
                 .with_coords({150.F, 150.F})
                 .with_altitude(100.F)
                 .with_attack_speed(10.F)
                 .with_acceleration_path(10.F)
                 .with_turn_threshold(0.3F)
                 .with_angular_speed(1.F)
                 .with_initial_direction(0.F)
                 .build();

  Ammo ammo{"VOG-17", 0.35F, 0.07F, 0.0F};
  Coord target{200.F, 200.F};

  Task result = as.solve(drone, ammo, target);

  std::cout << "Test";
}