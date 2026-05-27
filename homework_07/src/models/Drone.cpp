#include "models/Drone.hpp"
#include "models/DroneBuilder.hpp"

#include <cmath>

float Drone::calculate_drone_target_direction_delta(const Coord& target) const
{
  Coord diff = target - position_;

  float dir_to_target = std::atan2(diff.y_, diff.x_);
  float delta = dir_to_target - current_direction_;

  return std::atan2(std::sin(delta), std::cos(delta));
}

DroneBuilder Drone::builder()
{
  return DroneBuilder();
}

void Drone::increment_speed(float dt) {}
void Drone::increment_direction(const Coord& target, float dt) {}
void Drone::increment_position(float dt) {}
bool Drone::is_position_reached(const Coord& target, float threshold) const { return false; }
float Drone::penalty(const Coord& target) const { return 0.0F; }

