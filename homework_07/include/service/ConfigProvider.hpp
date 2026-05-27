#pragma once

#include "dto/ConfigContext.hpp"
#include "dto/AmmoContext.hpp"

#include "service/interface/IConfigProvider.hpp"

class ConfigProvider : public IConfigProvider {
  Drone* drone_;
  Ammo* ammo_;
  Simulation* simulation_;

  Ammo* fetch_by_name(const ConfigContext* config_context, const AmmoContext* ammo_context);

public:
  ConfigProvider(const ConfigContext* config_context, const AmmoContext* ammo_context);

  inline Drone* get_drone() const override { return drone_; }
  inline Ammo* get_ammo() const override { return ammo_; }
  inline Simulation* get_simulation() const override { return simulation_; }

  ~ConfigProvider() override;
};