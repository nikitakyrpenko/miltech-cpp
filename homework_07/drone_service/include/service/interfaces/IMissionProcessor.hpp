#pragma once

#include "models/SimulationStep.hpp"

class IMissionProccessor {
public:
  virtual SimulationStep step() = 0;
  virtual bool has_finished() = 0;

  virtual ~IMissionProccessor() = default;
};