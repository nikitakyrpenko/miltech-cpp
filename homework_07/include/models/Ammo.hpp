#pragma once

#include <string>

class Ammo {
  std::string name_{};
  float mass_{};
  float drag_{};
  float lift_{};

public:
  Ammo(const std::string&, float mass, float drag, float lift);
  float calculate_ammo_time_to_fall(float altitude, float on_speed) const;
  float calculate_ammo_distance_to_fall(float ammo_time_to_fall, float on_speed) const;
};
