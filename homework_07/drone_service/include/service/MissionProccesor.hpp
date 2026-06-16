#include "models/Ammo.hpp"
#include "models/Simulation.hpp"
#include "models/Task.hpp"
#include "service/interfaces/IBallisticSolutionEvaluator.hpp"
#include "service/interfaces/IFirepointProvider.hpp"
#include "service/interfaces/IBallisticSolver.hpp"
#include "service/interfaces/IMissionProcessor.hpp"
#include "service/interfaces/ITargetProvider.hpp"
#include "service/interfaces/IDroneProvider.hpp"
#include "service/TargetProviderIterator.hpp"

#include <memory>
#include <utility>
#include <vector>

class MissionProccessor : public IMissionProccessor {
  std::unique_ptr<IFirepointProvider> solver_;
  std::unique_ptr<ITargetProvider> target_provider_;
  std::unique_ptr<IDroneProvider> drone_provider_;
  std::unique_ptr<IBallisticSolutionEvaluator> evaluator_;
  std::unique_ptr<IBallisticSolver> fall_solver_;

  const Drone& drone_;
  const Ammo& ammo_;
  const Simulation& sim_;

  TargetProviderIterator current_target_;

  float clock_{0.0F};
  int current_target_id_{-1};  // target the drone is committed to (-1 = none); deadband keeps it stable

  const Task& select_task(const std::vector<Task>& tasks) const;
  SimulationStep build_step(const Task& chosen, float ammo_time_to_fall, float ammo_distance_to_fall) const;

public:
  MissionProccessor(std::unique_ptr<IFirepointProvider> solver,
                    std::unique_ptr<ITargetProvider> target_provider,
                    std::unique_ptr<IDroneProvider> drone_provider,
                    std::unique_ptr<IBallisticSolutionEvaluator> evaluator,
                    std::unique_ptr<IBallisticSolver> fall_solver)
    : solver_(std::move(solver))
    , target_provider_(std::move(target_provider))
    , drone_provider_(std::move(drone_provider))
    , evaluator_(std::move(evaluator))
    , fall_solver_(std::move(fall_solver))
    , drone_(drone_provider_->get_drone())
    , ammo_(drone_provider_->get_ammo())
    , sim_(drone_provider_->get_simulation())
    , current_target_(target_provider_->begin())
  {
  }

  SimulationStep step() override;
  bool has_finished() override;

  ~MissionProccessor() override = default;
};
