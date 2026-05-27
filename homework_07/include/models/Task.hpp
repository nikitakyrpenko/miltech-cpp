#pragma once
#include "Coord.hpp"

struct Task {
  Coord intermidiate_{};
  Coord fire_{};
  bool has_intermidiate_{};
  float time_taken{};
};
