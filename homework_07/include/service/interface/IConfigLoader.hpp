#pragma once

#include "dto/ConfigContext.hpp"
#include "dto/TargetContext.hpp"
#include "dto/AmmoContext.hpp"

class IConfigLoader {
public:
  virtual ConfigContext* get_config_context() = 0;
  virtual TargetContext* get_target_context() = 0;
  virtual AmmoContext* get_ammo_context() = 0;

  virtual ~IConfigLoader() = default;
};