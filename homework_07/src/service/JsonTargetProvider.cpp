#include "service/JsonTargetProvider.hpp"

#include <cmath>

Coord JsonTargetProvider::get_target(int target_id, float tick, float delta) const
{
  const Coord* coords = ctx_->targets_[target_id]->get_coords();
  float array_time_step = static_cast<float>(ctx_->array_time_step_);
  int time_steps = ctx_->time_steps_;

  int index = static_cast<int>(std::floor(tick / array_time_step)) % time_steps;
  int next_index = (index + 1) % time_steps;

  float frac = std::fmod(tick, array_time_step) / array_time_step;

  Coord curr = coords[index] + (coords[next_index] - coords[index]) * frac;

  if (delta == 0.0F) {
    return curr;
  }

  Coord velocity = (coords[next_index] - curr) / array_time_step;

  return curr + velocity * delta;
}
