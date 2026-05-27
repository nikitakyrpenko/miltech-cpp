#pragma once

#include "service/interface/IBallisticSolver.hpp"

class IMissionProccessor {
public:
  virtual void change_solver(const IBallisticSolver& ballistic_solver) = 0;
  virtual void run() = 0;

  virtual ~IMissionProccessor() = default;
};