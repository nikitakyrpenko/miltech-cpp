#pragma once
#include <cmath>

#include "Coord.hpp"

float calculate_turning_angle(const Coord& from, const Coord& to, float curr_direction)
{
  Coord diff = to - from;

  float dir_to_target = std::atan2(diff.y_, diff.x_);
  float delta = dir_to_target - curr_direction;

  return std::atan2(std::sin(delta), std::cos(delta));
}