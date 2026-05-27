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

void Drone::increment_speed(float dt)
{
  switch (state_) {
    case STOPPED:
      state_ = TURNING;
      return;
    case TURNING:
      current_speed_ = 0.0F;
      return;
    case MOVING:
      current_speed_ = attack_speed_;
      return;
    case ACCELERATING:
      current_speed_ += dt * acceleration();
      if (current_speed_ >= attack_speed_) {
        current_speed_ = attack_speed_;
        state_ = MOVING;
      }
      return;
    case DECELERATING:
      current_speed_ -= dt * acceleration();
      if (current_speed_ <= 0.0F) {
        current_speed_ = 0.0F;
        state_ = TURNING;
      }
      return;
  }
}

void Drone::increment_direction(const Coord& target, float dt)
{
  float delta = calculate_drone_target_direction_delta(target);

  if (state_ == MOVING) {
    if (std::abs(delta) > turn_threshold_)
      state_ = DECELERATING;
    else
      current_direction_ = std::atan2(target.y_ - position_.y_, target.x_ - position_.x_);
    return;
  }

  if (state_ != TURNING)
    return;

  if (std::abs(delta) <= turn_threshold_) {
    current_direction_ = std::atan2(target.y_ - position_.y_, target.x_ - position_.x_);
    state_ = ACCELERATING;
    return;
  }

  float rot_step = angular_speed_ * dt;
  if (std::abs(delta) <= rot_step) {
    current_direction_ += delta;
    state_ = ACCELERATING;
  }
  else {
    current_direction_ += (delta > 0.0F ? 1.0F : -1.0F) * rot_step;
  }
}

void Drone::increment_position(float dt)
{
  position_.x_ += current_speed_ * std::cos(current_direction_) * dt;
  position_.y_ += current_speed_ * std::sin(current_direction_) * dt;
}

bool Drone::is_position_reached(const Coord& target, float threshold) const
{
  Coord diff = target - position_;
  return (diff.x_ * diff.x_ + diff.y_ * diff.y_) <= (threshold * threshold);
}

float Drone::penalty(const Coord& target) const
{
  float dir = std::abs(calculate_drone_target_direction_delta(target));

  switch (state_) {
    case STOPPED:
    case TURNING:
      return dir <= turn_threshold_ ? 0.0F : dir / angular_speed_;
    case ACCELERATING:
    case DECELERATING: {
      float t = current_speed_ / acceleration();
      return dir <= turn_threshold_ ? t : t + dir / angular_speed_;
    }
    case MOVING: {
      float t = attack_speed_ / acceleration();
      return dir <= turn_threshold_ ? t : t + dir / angular_speed_;
    }
  }
  return 0.0F;
}
