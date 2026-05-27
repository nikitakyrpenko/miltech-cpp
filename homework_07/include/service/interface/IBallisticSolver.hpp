#pragma once

#include "models/Coord.hpp"
#include "models/Drone.hpp"
#include "models/Task.hpp"
#include "models/Ammo.hpp"

class IBallisticSolver {
public:
  virtual Task solve(const Drone& drone, const Ammo& ammo, const Coord& target) const = 0;
  virtual ~IBallisticSolver() = default;
};
