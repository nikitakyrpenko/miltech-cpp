
#include "models/SimulationStep.hpp"
#include "models/Task.hpp"
#include "service/MissionProccessor.hpp"

#include <vector>

MissionProccessor::MissionProccessor(const IBallisticSolver* solver,
                                     const ITargetProvider* target_provider,
                                     const IConfigProvider* config_provider)
  : solver_(solver)
  , target_provider_(target_provider)
  , config_provider_(config_provider)
  , drone_(config_provider->get_drone())
{
}

MissionProccessor::~MissionProccessor(){};

void MissionProccessor::change_solver(const IBallisticSolver& solver)
{
  solver_ = &solver;
}

MissionProccessor::TargetTask MissionProccessor::calculate_task(
  const Drone& d, const Ammo& a, int target_id, float tick, float ammo_time_to_fall)
{
  Coord initial = target_provider_->get_target(target_id, tick);
  Task t = solver_->solve(d, a, initial);
  Coord predicted = target_provider_->get_target(target_id, tick, t.time_taken + ammo_time_to_fall);
  return {target_id, solver_->solve(d, a, predicted)};
}

MissionProccessor::TargetTask MissionProccessor::find_optimal(std::vector<TargetTask> tasks)
{
  int idx = 0;
  for (int i = 1; i < static_cast<int>(tasks.size()); i++) {
    if (tasks[i].task_.time_taken < tasks[idx].task_.time_taken)
      idx = i;
  }
  return tasks[idx];
}

MissionProccessor::TargetTask MissionProccessor::step(float tick)
{
  const Ammo& ammo = *config_provider_->get_ammo();
  float ammo_time_to_fall = ammo.calculate_ammo_time_to_fall(drone_->get_altitude(), drone_->get_attack_speed());

  std::vector<TargetTask> tasks(target_provider_->get_size());
  for (int i = 0; i < target_provider_->get_size(); i++) {
    tasks[i] = calculate_task(*drone_, ammo, i, tick, ammo_time_to_fall);
  }

  return find_optimal(tasks);
}

std::vector<SimulationStep> MissionProccessor::run()
{
  const Simulation* sim = config_provider_->get_simulation();

  TargetTask current_task{};
  std::vector<SimulationStep> steps{};

  int iter = 0;
  float tick = sim->time_step_;

  while (iter < MAX_ITER) {
    TargetTask optimal_task = step(tick);

    if (current_task.target_id == -1 || current_task.target_id == optimal_task.target_id) {
      current_task.target_id = optimal_task.target_id;
      current_task.task_ = optimal_task.task_;
    }
    // check worth switching
    else {
      Coord candidate;
      if (optimal_task.task_.has_intermidiate_) {
        candidate = optimal_task.task_.intermidiate_;
      }
      else {
        candidate = optimal_task.task_.fire_;
      }

      float switch_penalty = drone_->penalty(candidate);

      if (optimal_task.task_.time_taken + switch_penalty < current_task.task_.time_taken)
        current_task = optimal_task;
    }

    // check does drone reach intermidiate position
    if (!current_task.visited_intermediate_) {
      if (!current_task.task_.has_intermidiate_) {
        current_task.visited_intermediate_ = true;
      }
      else if (drone_->is_position_reached(current_task.task_.intermidiate_, 0.5F)) {
        current_task.visited_intermediate_ = true;
      }
    }

    Coord navigating_toward;
    if (current_task.visited_intermediate_) {
      navigating_toward = current_task.task_.fire_;
    }
    else {
      navigating_toward = current_task.task_.intermidiate_;
    }

    drone_->increment_speed(sim->time_step_);
    drone_->increment_direction(navigating_toward, sim->time_step_);
    drone_->increment_position(sim->time_step_);

    Coord predicted_target = target_provider_->get_target(current_task.target_id, tick);

    steps.push_back({
      .target_id_ = current_task.target_id,
      .direction_ = drone_->get_current_direction(),
      .state_ = drone_->get_state(),
      .position_ = drone_->get_position(),
      .drop_point_ = navigating_toward,
      .predicted_target_ = predicted_target,
    });

    if (current_task.visited_intermediate_ && drone_->is_position_reached(predicted_target, sim->hit_radius_)) {
      break;
    }

    tick += sim->time_step_;
    iter++;
  }

  return steps;
}
