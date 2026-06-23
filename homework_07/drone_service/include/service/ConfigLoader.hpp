#pragma once

#include "service/interfaces/IConfigLoader.hpp"

#include <memory>
#include <stdexcept>

class ConfigLoader : public IConfigLoader {
  std::unique_ptr<ConfigDTO> config_;
  std::unique_ptr<AmmoDTO> arsenal_;
  std::unique_ptr<TargetDTO> targets_;
  std::unique_ptr<BallisticTableDTO> table_;

public:
  ConfigLoader(const char* config_source, const char* ammo_source, const char* target_source, const char* table_source = nullptr);

  const ConfigDTO& get_config() const override { return *config_; }
  const AmmoDTO& get_arsenal() const override { return *arsenal_; }
  const TargetDTO& get_targets() const override { return *targets_; }
  const BallisticTableDTO& get_table() const override
  {
    if (!table_) {
      throw std::runtime_error("ConfigLoader: table was not loaded");
    }
    return *table_;
  }

  ~ConfigLoader() override;
};
