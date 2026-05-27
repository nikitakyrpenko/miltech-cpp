#pragma once

// remove drone from here
#include "models/Drone.hpp"
// #include "dto/AmmoContext.hpp"

#include <string>

struct ConfigContext {
  Drone* drone_;
  // AmmoContext* ammo_context_;
  std::string ammo_;
  float target_array_timestep_;
  float time_step_;
  float hit_radius_;
};