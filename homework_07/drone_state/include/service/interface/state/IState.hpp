#pragma once

#include "models/Coord.hpp"
#include "models/Drone.hpp"

#include <string>

class IState {
public:
  virtual const IState* execute(Drone& drone, const Coord& coord, float dt, bool should_decelerate = false) const = 0;
  virtual std::string name() const = 0;

  virtual ~IState() = default;
};