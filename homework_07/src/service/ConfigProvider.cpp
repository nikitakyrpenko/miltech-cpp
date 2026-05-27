#include "service/ConfigProvider.hpp"

#include <iostream>

Ammo* ConfigProvider::fetch_by_name(const ConfigContext* config_context, const AmmoContext* ammo_context)
{
  for (int i = 0; i < ammo_context->size; i++) {
    if (ammo_context->ammos_[i]->get_name() == config_context->ammo_) {
      return ammo_context->ammos_[i];
    }
  }
  std::cerr << "No such ammo type : " << config_context->ammo_ << std::endl;
  return nullptr;
}

ConfigProvider::ConfigProvider(const ConfigContext* config_context, const AmmoContext* ammo_context)
{
  drone_ = config_context->drone_;
  simulation_ = new Simulation{config_context->time_step_, config_context->hit_radius_};
  ammo_ = fetch_by_name(config_context, ammo_context);
}

ConfigProvider::~ConfigProvider()
{
  delete simulation_;
}