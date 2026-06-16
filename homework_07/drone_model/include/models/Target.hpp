#pragma once

#include "Coord.hpp"

#include <vector>

class Target {
  const int target_id_;
  const int time_step_;
  const int array_time_step_;
  const std::vector<Coord> coords_;

public:
  Target(const int id, std::vector<Coord> coords, int time_step, int array_time_step)
    : target_id_(id)
    , time_step_(time_step)
    , array_time_step_(array_time_step)
    , coords_(coords)
  {
  }

  inline int get_target_id() const { return target_id_; }
  inline int get_time_step() const { return time_step_; }
  inline int get_array_time_step() const { return array_time_step_; }

  const Coord approximate(float dt, float delta) const;
};
