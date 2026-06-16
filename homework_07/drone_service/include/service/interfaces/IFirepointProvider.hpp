#pragma once

#include "models/Drone.hpp"
#include "models/Task.hpp"
#include "service/interfaces/IBallisticSolver.hpp"

class IFirepointProvider {
public:
  virtual const BallisticSolution solve(const Drone& drone, const FallResult& fall, const Coord& target) const = 0;
  virtual ~IFirepointProvider() = default;
};