#pragma once

#include "DroneLink.hpp"
#include "UartLink.hpp"
#include "dto/AmmoDTO.hpp"
#include "dto/ConfigDTO.hpp"
#include "dto/TargetDTO.hpp"
#include "dto/BallisticTableDTO.hpp"
#include "service/interfaces/IConfigLoader.hpp"

#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <thread>
#include <chrono>

class UartConfigLoader : public IConfigLoader {
  ConfigDTO config_;
  AmmoDTO arsenal_;

public:
  UartConfigLoader(std::shared_ptr<UartLink> link)
  {
    auto& ammo_q = link->ammo_channel();
    auto& cfg_q = link->config_channel();

    while (true) {
      const auto packets = ammo_q.drain_all();
      if (!packets.empty()) {
        const auto& ammo = packets.front();
        config_.ammo_ = std::string(ammo.name);
        config_.hit_radius_ = ammo.hitRadius;
        arsenal_.ammos_.push_back({std::string(ammo.name), ammo.mass, ammo.drag, ammo.lift});
        std::cout << "Ammo fetched" << std::endl;
      }

      std::optional<dlink::DroneCfg> cfg = cfg_q.drain_to_last();
      if (cfg.has_value()) {
        config_.attack_speed_ = cfg->attackSpeed;
        config_.acceleration_path_ = cfg->accelerationPath;
        config_.angular_speed_ = cfg->angularSpeed;
        config_.turn_threshold_ = cfg->turnThreshold;
        config_.time_step_ = cfg->timeStep;
        config_.timescale = cfg->timeScale;
        std::cout << "Config fetched" << std::endl;
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
