#pragma once
#include "Coord.hpp"

class Target {
  int target_id_;
  int array_time_step_;
  int time_steps_;
  Coord* coords_;

public:
  int get_target_id() const;

  Coord approximate_at_t(float tick) const;
  Coord interpolate_by_time_delta(float tick, float delta) const;
};
