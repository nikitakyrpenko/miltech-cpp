#pragma once

#include "service/interface/IConfigLoader.hpp"

class JsonConfigLoader : public IConfigLoader {
  ConfigContext* config_;
  AmmoContext* ammo_;
  TargetContext* target_;

  JsonConfigLoader(ConfigContext* config, AmmoContext* ammo, TargetContext* target)
    : config_(config)
    , ammo_(ammo)
    , target_(target)
  {
  }

  static ConfigContext* load_config(const char* source);
  static AmmoContext* load_arsenal(const char* source);
  static TargetContext* load_targets(const char* source, int array_time_step);

public:
  static JsonConfigLoader* create(const char* sim, const char* ammo, const char* target);

  inline ConfigContext* get_config_context() override { return config_; }
  inline TargetContext* get_target_context() override { return target_; }
  inline AmmoContext* get_ammo_context() override { return ammo_; }

  ~JsonConfigLoader() override;
};