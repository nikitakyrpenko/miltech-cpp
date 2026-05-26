#include "service/interface/IBallisticSolver.hpp"
#include "service/interface/ITargetProvider.hpp"
#include "service/interface/IConfigProvider.hpp"

class IMissionProccessor {
  virtual void init(const ITargetProvider& target_provider,
                    const IBallisticSolver& ballistic_solver,
                    const IConfigProvider config_provider) = 0;
  virtual void change_solver(const IBallisticSolver& ballistic_solver) = 0;

  virtual ~IMissionProccessor() = default;
};