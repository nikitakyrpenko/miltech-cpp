#pragma once
#include <cmath>

#include "models/Coord.hpp"

namespace Calc {

float calculate_turning_angle(const Coord& from, const Coord& to, float curr_direction)
{
  Coord diff = to - from;

  float dir_to_target = std::atan2(diff.y_, diff.x_);
  float delta = dir_to_target - curr_direction;

  return std::atan2(std::sin(delta), std::cos(delta));
}

float angle(const Coord& from, const Coord& to)
{
  return std::atan2(to.y_ - from.y_, to.x_ - from.x_);
}

float lenght(const Coord& from, const Coord& to)
{
  float xd = to.x_ - from.x_;
  float yd = to.y_ - from.y_;

  return std::hypot(xd, yd);
}
}  // namespace Calc