#pragma once

#include "models/Coord.hpp"

#include <string>

struct ConfigDTO {
  Coord position_;
  float altitude_;
  float initial_direction_;
  float attack_speed_;
  float acceleration_path_;
  float angular_speed_;
  float turn_threshold_;
  std::string ammo_;
  float target_array_timestep_;
  float time_step_;
  float hit_radius_;
};