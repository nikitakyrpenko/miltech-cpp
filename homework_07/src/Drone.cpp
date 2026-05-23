#include "Drone.hpp"
#include "DroneBuilder.hpp"

DroneBuilder Drone::builder()
{
  return DroneBuilder();
}

const Coord& Drone::getPosition() const
{
  return position_;
}

float Drone::getAltitude() const
{
  return altitude_;
}

float Drone::getInitialDirection() const
{
  return initial_direction_;
}

float Drone::getAttackSpeed() const
{
  return attack_speed_;
}

float Drone::getAccelerationPath() const
{
  return acceleration_path_;
}

float Drone::getAngularSpeed() const
{
  return angular_speed_;
}

float Drone::getTurnThreshold() const
{
  return turn_threshold_;
}

float Drone::getCurrentSpeed() const
{
  return current_speed_;
}

float Drone::getCurrentDirection() const
{
  return current_direction_;
}

State Drone::getState() const
{
  return state_;
}
