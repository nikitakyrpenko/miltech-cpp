#include <iostream>

#include "DroneBuilder.hpp"

int main()
{
  auto d = Drone::builder().with_coords({10, 20}).build();
}