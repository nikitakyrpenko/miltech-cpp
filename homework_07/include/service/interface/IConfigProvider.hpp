#pragma once

#include "models/Ammo.hpp"
#include "models/Drone.hpp"
#include "models/Simulation.hpp"

class IConfigProvider {
public:
  virtual Drone* get_drone() const = 0;
  virtual Ammo* get_ammo() const = 0;
  virtual Simulation* get_simulation() const = 0;

  virtual ~IConfigProvider() = default;
};