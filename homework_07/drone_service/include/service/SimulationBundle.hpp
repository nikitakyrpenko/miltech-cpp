#pragma once

#include "service/DronePhysics.hpp"
#include "service/MissionProccesor.hpp"
#include "service/TargetProvider.hpp"

#include <memory>

struct SimulationBundle {
  std::shared_ptr<TargetProvider> target;
  std::shared_ptr<DronePhysics> physics;
  std::unique_ptr<MissionProccessor> mission;
};
