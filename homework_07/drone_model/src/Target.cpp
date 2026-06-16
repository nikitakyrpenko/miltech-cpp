#include "models/Target.hpp"

#include <cmath>

const Coord Target::approximate(float dt, float delta = 0.F) const
{
  const float ats = static_cast<float>(array_time_step_);

  const int index = static_cast<int>(std::floor(dt / ats)) % time_step_;
  const int next_index = (index + 1) % time_step_;
  const float frac = std::fmod(dt, ats) / ats;

  const Coord curr = coords_.at(index) + (coords_.at(next_index) - coords_.at(index)) * frac;

  if (delta == 0.0F) {
    return curr;
  }

  const Coord velocity = (coords_.at(next_index) - curr) / ats;

  // sanity border on delta value
  const float horizon = ats * static_cast<float>(time_step_);
  const float lead = (delta > horizon) ? horizon : delta;

  return curr + velocity * lead;
}