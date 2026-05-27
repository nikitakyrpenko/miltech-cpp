#pragma once

#include "models/Coord.hpp"
#include "models/Drone.hpp"

struct SimulationStep {
  int target_id_;
  float direction_;
  State state_;
  Coord position_;
  Coord drop_point_;
  Coord aim_point_;
  Coord predicted_target_;
};