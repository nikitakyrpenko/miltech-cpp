#pragma once

#include "UartLink.hpp"
#include "dto/AmmoDTO.hpp"
#include "dto/ConfigDTO.hpp"
#include "dto/TargetDTO.hpp"
#include "dto/BallisticTableDTO.hpp"
#include "service/interfaces/IConfigLoader.hpp"

#include <memory>
#include <stdexcept>
#include <thread>
#include <chrono>

class UartConfigLoader : public IConfigLoader {
  ConfigDTO config_;
  AmmoDTO arsenal_;

public:
  // Blocks until PKT_AMMO arrives on link->ammo_channel().
  // PKT_CONFIG is not sent by the checker in practice, so attack_speed_/acceleration_path_/
  // angular_speed_/turn_threshold_/time_step_/timescale stay whatever base_config (config.json) had.
  UartConfigLoader(std::shared_ptr<UartLink> link, const ConfigDTO& base_config)
    : config_(base_config)
  {
    auto& ammo_q = link->ammo_channel();

    while (true) {
      const auto packets = ammo_q.drain_all();
      if (!packets.empty()) {
        const auto& ammo = packets.front();
        config_.ammo_ = std::string(ammo.name);
        config_.hit_radius_ = ammo.hitRadius;
        arsenal_.ammos_.push_back({std::string(ammo.name), ammo.mass, ammo.drag, ammo.lift});
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  const ConfigDTO& get_config() const override { return config_; }
  const AmmoDTO& get_arsenal() const override { return arsenal_; }
  const TargetDTO& get_targets() const override { throw std::logic_error("not available in UART mode"); }
  const BallisticTableDTO& get_table() const override { throw std::logic_error("not available in UART mode"); }
};
