#pragma once

#include "dto/AmmoDTO.hpp"
#include "dto/ConfigDTO.hpp"
#include "dto/TargetDTO.hpp"
#include "dto/BallisticTableDTO.hpp"

class IConfigLoader {
public:
  virtual const ConfigDTO& get_config() const = 0;
  virtual const AmmoDTO& get_arsenal() const = 0;
  virtual const TargetDTO& get_targets() const = 0;
  virtual const BallisticTableDTO& get_table() const = 0;

  // Blocks calling thread until this loader has all data required for get_config()/get_arsenal()
  // to be valid. No-op by default — synchronous loaders (e.g. from a file) have nothing to wait for.
  virtual void wait_ready() const {}

  virtual ~IConfigLoader() = default;
};
