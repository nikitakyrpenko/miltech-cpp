#pragma once

#include "models/SimulationStep.hpp"
#include "service/interface/IBallisticSolver.hpp"

#include <vector>

class IMissionProccessor {
public:
  virtual void change_solver(const IBallisticSolver& ballistic_solver) = 0;
  virtual std::vector<SimulationStep> run() = 0;

  virtual ~IMissionProccessor() = default;
};