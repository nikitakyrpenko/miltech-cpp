#pragma once

#include "service/interface/IBallisticSolver.hpp"
#include "service/interface/IMissionProccesor.hpp"
#include "service/interface/ITargetProvider.hpp"
#include "service/interface/IConfigProvider.hpp"

#include <vector>

class MissionProccessor : public IMissionProccessor {
  struct TargetTask {
    int target_id{-1};
    Task task_{};
    bool visited_intermediate_{false};
    Coord predicted_target_{};
  };
  const IBallisticSolver* solver_;
  const ITargetProvider* target_provider_;
  const IConfigProvider* config_provider_;

  // mutation pointer, cannot change drone outside MissionProccessor
  Drone* drone_;

  static constexpr int MAX_ITER = 10000;

  TargetTask calculate_task(const Drone& d, const Ammo& a, int target_id, float tick, float ammo_time_to_fall);
  TargetTask find_optimal(std::vector<TargetTask> tasks);
  TargetTask step(float tick);

public:
  MissionProccessor(const IBallisticSolver* solver, const ITargetProvider* target_provider, const IConfigProvider* config_provider);
  ~MissionProccessor() override;

  std::vector<SimulationStep> run() override;
  void change_solver(const IBallisticSolver& solver) override;
};