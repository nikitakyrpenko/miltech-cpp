#include "service/MissionFactory.hpp"
#include "service/AnalyticalSolver.hpp"
#include "service/ConfigProvider.hpp"
#include "service/JsonConfigLoader.hpp"
#include "service/TargetProvider.hpp"

Mission* MissionFactory::create(
  LoaderType loader_type, SolverType solver_type, const char* sim_file, const char* ammo_file, const char* target_file)
{
  IConfigLoader* loader = nullptr;
  if (loader_type == LoaderType::JSON)
    loader = JsonConfigLoader::create(sim_file, ammo_file, target_file);
  if (!loader)
    return nullptr;

  IConfigProvider* config = new ConfigProvider(loader->get_config_context(), loader->get_ammo_context());
  ITargetProvider* targets = new TargetProvider(loader->get_target_context());

  IBallisticSolver* solver = nullptr;
  if (solver_type == SolverType::ANALYTICAL)
    solver = new AnalyticalSolver();

  return new Mission{loader, config, targets, solver, new MissionProccessor(solver, targets, config)};
}
