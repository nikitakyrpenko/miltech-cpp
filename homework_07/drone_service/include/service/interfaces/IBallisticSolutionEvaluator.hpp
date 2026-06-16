#pragma once

#include "models/Drone.hpp"
#include "models/Task.hpp"

class IBallisticSolutionEvaluator {
public:
  virtual const Task calculate_time_taken(const Drone& drone, int target_id, const BallisticSolution& solution) const = 0;

  virtual ~IBallisticSolutionEvaluator() = default;
};
