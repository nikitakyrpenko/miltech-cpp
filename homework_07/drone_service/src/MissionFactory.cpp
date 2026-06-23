#include "service/MissionFactory.hpp"

#include "service/AnalyticalBallisticSolver.hpp"
#include "service/BallisticSolutionEvaluator.hpp"
#include "service/ConfigLoader.hpp"
#include "service/DronePhysics.hpp"
#include "service/FirepointProvider.hpp"
#include "service/MissionProccesor.hpp"
#include "service/TableBallisticSolver.hpp"
#include "service/TargetProvider.hpp"

#include "ThreadSafeQueue.hpp"
#include "models/DroneCommand.hpp"

#include <memory>
#include <stdexcept>

namespace {

std::shared_ptr<IConfigLoader> make_loader(
  LoaderType type, const char* config_source, const char* ammo_source, const char* target_source, const char* table_source)
{
  switch (type) {
    case LoaderType::JSON:
      return std::make_shared<ConfigLoader>(config_source, ammo_source, target_source, table_source);
  }
  throw std::runtime_error("MissionFactory: unsupported loader type");
}

std::unique_ptr<IBallisticSolver> make_fall_solver(SolverType type, const std::shared_ptr<IConfigLoader>& loader)
{
  switch (type) {
    case SolverType::ANALYTICAL:
      return std::make_unique<AnalyticalBallisticSolver>();
    case SolverType::TABLE:
      return std::make_unique<TableBallisticSolver>(loader);
  }
  throw std::runtime_error("MissionFactory: unsupported solver type");
}

}  // namespace

SimulationBundle MissionFactory::create(LoaderType loader_type,
                                        SolverType solver_type,
                                        const char* config_source,
                                        const char* ammo_source,
                                        const char* target_source,
                                        const char* table_source) const
{
  std::shared_ptr<IConfigLoader> loader = make_loader(loader_type, config_source, ammo_source, target_source, table_source);

  auto channel = std::make_shared<SynchronizedQueue<DroneCommand>>();

  auto target = std::make_shared<TargetProvider>(loader);
  auto physics = std::make_shared<DronePhysics>(*loader, channel);

  auto mission = std::make_unique<MissionProccessor>(target,
                                                     physics,
                                                     loader,
                                                     make_fall_solver(solver_type, loader),
                                                     std::make_unique<FirepointProvider>(),
                                                     std::make_unique<BallisticSolutionEvaluator>(),
                                                     channel);

  return SimulationBundle{std::move(target), std::move(physics), std::move(mission)};
}
