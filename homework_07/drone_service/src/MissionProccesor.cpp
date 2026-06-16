#include "Calc.hpp"
#include "models/Coord.hpp"
#include "models/Drone.hpp"
#include "models/SimulationStep.hpp"
#include "models/Task.hpp"
#include "service/MissionProccesor.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

constexpr float SWITCH_FACTOR = 0.5F;  // switch only if a rival costs <= this fraction of the committed task (50% better)
constexpr float LOCK_FACTOR = 2.0F;

bool MissionProccessor::has_finished()
{
  return drone_provider_->has_task_completed();
}

const Task& MissionProccessor::select_task(const std::vector<Task>& tasks) const
{
  const Task& optimal =
    *std::min_element(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) { return a.time_taken < b.time_taken; });

  const auto current = std::find_if(tasks.begin(), tasks.end(), [this](const Task& t) { return t.id_ == current_target_id_; });

  if (current == tasks.end()) {
    return optimal;
  }

  // check is lock on target needed, if prerequisits are good, keep current
  const bool lock_in = Calc::lenght(drone_.get_position(), current->solution_.fire_) <= LOCK_FACTOR * sim_.hit_radius_;

  // check is margin to switch is more then 50% to reduce states jittering
  const bool beat_margin = optimal.time_taken < current->time_taken * SWITCH_FACTOR;

  return (lock_in || !beat_margin) ? *current : optimal;
}

SimulationStep MissionProccessor::step()
{
  // TODO: can be precomputed in ctor
  const FallResult ammo_fall_parameters = fall_solver_->fall(ammo_, drone_.get_altitude(), drone_.get_attack_speed());

  clock_ += sim_.time_step_;

  std::vector<Task> tasks{};
  tasks.reserve(target_provider_->size());

  for (auto cursor = target_provider_->begin(); cursor.has_next(); cursor.next()) {
    const Target& target = cursor.get();
    const int id = target.get_target_id();

    const Task task =
      evaluator_->calculate_time_taken(drone_, id, solver_->solve(drone_, ammo_fall_parameters, target.approximate(clock_, 0.F)));

    if (!std::isfinite(task.time_taken) || task.time_taken == std::numeric_limits<float>::max()) {
      continue;
    }

    const float lead = task.time_taken + ammo_fall_parameters.time;
    tasks.emplace_back(
      evaluator_->calculate_time_taken(drone_, id, solver_->solve(drone_, ammo_fall_parameters, target.approximate(clock_, lead))));
  }

  // TODO : maybe it can break simulator revisit it
  if (tasks.empty()) {
    return {};
  }

  const Task& chosen = select_task(tasks);
  current_target_id_ = chosen.id_;

  drone_provider_->execute(chosen, sim_.time_step_);

  return build_step(chosen, ammo_fall_parameters.time, ammo_fall_parameters.distance);
}

SimulationStep MissionProccessor::build_step(const Task& chosen, float ammo_time_to_fall, float ammo_distance_to_fall) const
{
  const Coord predicted_target = target_provider_->get_target(chosen.id_).approximate(clock_, chosen.time_taken + ammo_time_to_fall);
  const Coord aim_point = drone_.get_position() +
                          Coord{std::cos(drone_.get_current_direction()), std::sin(drone_.get_current_direction())} * ammo_distance_to_fall;

  return SimulationStep{
    .target_id_ = chosen.id_,
    .direction_ = drone_.get_current_direction(),
    .state_ = drone_provider_->get_current_state(),
    .position_ = drone_.get_position(),
    .drop_point_ = drone_provider_->get_active_coord(),
    .aim_point_ = aim_point,
    .predicted_target_ = predicted_target,
  };
}