#include "Drone.hpp"
#include <cmath>
#include "DroneBuilder.hpp"

float Drone::acceleration() const
{
  return (attack_speed_ * attack_speed_) / (2.0F * acceleration_path_);
}

void Drone::increment_position(float tick)
{
  position_.x_ += current_speed_ * std::cos(current_direction_) * tick;
  position_.y_ += current_speed_ * std::sin(current_direction_) * tick;
}

void Drone::increment_speed(float tick)
{
  if (state_ == ACCELERATING) {
    current_speed_ += tick * acceleration();
    return;
  }
  if (state_ == DECELERATING) {
    current_speed_ -= tick * acceleration();
    return;
  }
  if (state_ == MOVING) {
    current_speed_ = attack_speed_;
    return;
  }
}

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

const Coord& Drone::get_position() const
{
  return position_;
}

float Drone::get_altitude() const
{
  return altitude_;
}

float Drone::get_initial_direction() const
{
  return initial_direction_;
}

float Drone::get_attack_speed() const
{
  return attack_speed_;
}

float Drone::get_acceleration_path() const
{
  return acceleration_path_;
}

float Drone::get_angular_speed() const
{
  return angular_speed_;
}

float Drone::get_turn_threshold() const
{
  return turn_threshold_;
}

float Drone::get_current_speed() const
{
  return current_speed_;
}

float Drone::get_current_direction() const
{
  return current_direction_;
}

State Drone::get_state() const
{
  return state_;
}
