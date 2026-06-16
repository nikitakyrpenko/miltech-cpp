#pragma once

#include <vector>

struct BallisticTable {
  struct Result {
    float ammo_time_to_fall;
    float ammo_distance_to_fall;
  };

  std::vector<float> z, v, d, m, l;
  std::vector<Result> data;
};