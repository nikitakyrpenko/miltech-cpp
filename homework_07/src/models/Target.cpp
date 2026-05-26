#include "models/Target.hpp"

#include <cmath>

int Target::get_target_id() const
{
  return target_id_;
}

Coord Target::approximate_at_t(float tick) const
{
  int index = static_cast<int>(std::floor(tick / array_time_step_)) % time_steps_;
  int next_index = (index + 1) % time_steps_;

  float frac = std::fmod(tick, array_time_step_) / array_time_step_;

  return coords_[index] + (coords_[next_index] - coords_[index]) * frac;
}

Coord Target::interpolate_by_time_delta(float tick, float delta) const
{
  int index = static_cast<int>(std::floor(tick / array_time_step_)) % time_steps_;
  int next_index = (index + 1) % time_steps_;

  float frac = std::fmod(tick, array_time_step_) / array_time_step_;

  Coord curr = coords_[index] + (coords_[next_index] - coords_[index]) * frac;
  Coord next = coords_[next_index];

  Coord velocity = (next - curr) / array_time_step_;

  return curr + velocity * delta;
}