#pragma once
#include "models/Target.hpp"

struct TargetContext {
  Target** targets_;
  int target_count_;
  int time_steps_;
  int array_time_step_;
};