#include "models/Ammo.hpp"

#include <cmath>

static constexpr float Grav = 9.81F;

Ammo::Ammo(const std::string& name, float mass, float drag, float lift)
  : name_(name)
  , mass_(mass)
  , drag_(drag)
  , lift_(lift)
{
}

float Ammo::calculate_ammo_time_to_fall(float altitude, float on_speed) const
{
  float a = (drag_ * Grav * mass_) - (2.0f * drag_ * drag_ * lift_ * on_speed);
  float b = (-3.0f * Grav * mass_ * mass_) + (3.0f * drag_ * lift_ * mass_ * on_speed);
  float c = 6.0f * mass_ * mass_ * altitude;

  float p = (-b * b) / (3.0f * a * a);
  float q = (2.0f * b * b * b) / (27.0f * a * a * a) + (c / a);

  float acos_arg = (3.0f * q) / (2.0f * p) * std::sqrt(-3.0f / p);
  if (acos_arg < -1.0f || acos_arg > 1.0f) {
    return 0.0f;
  }

  return 2.0f * std::sqrt(-p / 3.0f) * std::cos((std::acos(acos_arg) + 4.0f * std::acos(-1.0f)) / 3.0f) - b / (3.0f * a);
}

float Ammo::calculate_ammo_distance_to_fall(float ammo_time_to_fall, float on_speed) const
{
  if (ammo_time_to_fall == 0.0f) {
    return 0.0f;
  }

  float distance_to_fall =
    (std::pow(ammo_time_to_fall, 3) *
     ((6.0f * drag_ * Grav * lift_ * mass_) - (6.0f * std::pow(drag_, 2) * (std::pow(lift_, 2) - 1) * on_speed))) /
      (36.0f * std::pow(mass_, 2)) +
    (std::pow(ammo_time_to_fall, 5) * ((3.0f * std::pow(drag_, 3) * Grav * std::pow(lift_, 3) * mass_) -
                                       (3.0f * std::pow(drag_, 4) * std::pow(lift_, 2) * (std::pow(lift_, 2) + 1) * on_speed))) /
      (36.0f * (std::pow(lift_, 2) + 1) * std::pow(mass_, 4)) +
    (std::pow(ammo_time_to_fall, 4) *
     ((3.0f * std::pow(drag_, 3) * (std::pow(lift_, 2) + 1) * std::pow(lift_, 2) * on_speed) +
      (6.0f * std::pow(drag_, 3) * (std::pow(lift_, 2) + 1) * std::pow(lift_, 4) * on_speed) -
      (6.0f * std::pow(drag_, 2) * Grav * (std::pow(lift_, 4) + std::pow(lift_, 2) + 1) * lift_ * mass_))) /
      (36.0f * std::pow(std::pow(lift_, 2) + 1, 2) * std::pow(mass_, 3)) -
    (drag_ * std::pow(ammo_time_to_fall, 2) * on_speed) / (2.0f * mass_) + (ammo_time_to_fall * on_speed);

  return distance_to_fall;
}
