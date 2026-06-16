#pragma once

#include "models/Coord.hpp"

#include <string>

struct SimulationStep {
  int target_id_;
  float direction_;
  std::string state_;
  Coord position_;
  Coord drop_point_;
  Coord aim_point_;
  Coord predicted_target_;
};