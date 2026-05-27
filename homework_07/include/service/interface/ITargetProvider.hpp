#pragma once

#include "models/Coord.hpp"

class ITargetProvider {
public:
  virtual Coord get_target(int target_id, float tick, float delta = 0.0F) const = 0;
  virtual int get_size() const = 0;

  virtual ~ITargetProvider() = default;
};