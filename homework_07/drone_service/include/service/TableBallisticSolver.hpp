#pragma once

#include "service/interfaces/IConfigLoader.hpp"
#include "service/interfaces/IBallisticSolver.hpp"

#include <memory>

class TableBallisticSolver : public IBallisticSolver {
  std::shared_ptr<IConfigLoader> loader_;

public:
  TableBallisticSolver(std::shared_ptr<IConfigLoader> loader)
    : loader_(std::move(loader))
  {
  }

  FallResult fall(const Ammo& ammo, float altitude, float speed) const override;
};
