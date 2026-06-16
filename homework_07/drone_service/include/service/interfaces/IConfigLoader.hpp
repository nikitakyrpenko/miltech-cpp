#pragma once

#include "dto/AmmoDTO.hpp"
#include "dto/ConfigDTO.hpp"
#include "dto/TargetDTO.hpp"
#include "dto/BallisticTable.hpp"

class IConfigLoader {
public:
  virtual const ConfigDTO& get_config() const = 0;
  virtual const AmmoDTO& get_arsenal() const = 0;
  virtual const TargetDTO& get_targets() const = 0;
  virtual const BallisticTable& get_table() const = 0;

  virtual ~IConfigLoader() = default;
};
