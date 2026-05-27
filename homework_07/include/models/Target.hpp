#pragma once
#include "Coord.hpp"

class Target {
  int target_id_;
  Coord* coords_;

public:
  Target(int id, Coord* coords)
    : target_id_(id)
    , coords_(coords)
  {
  }

  ~Target() { delete[] coords_; }

  inline int get_target_id() const { return target_id_; }
  const Coord* get_coords() const { return coords_; }
};
