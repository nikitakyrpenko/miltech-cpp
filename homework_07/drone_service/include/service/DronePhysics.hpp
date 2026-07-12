#pragma once

#include "ThreadSafeQueue.hpp"

#include "service/interfaces/IDronePhysics.hpp"
#include "service/interfaces/IConfigLoader.hpp"

#include <atomic>
#include <cmath>
#include <latch>
#include <memory>
#include <mutex>
#include <thread>

class DronePhysics : public IDronePhysics {
  mutable SynchronizedQueue<DroneCommand> channel_;

  const float physics_time_step{0.1F};
  const float timescale_{1.0F};

  DroneTelemetry telemetry_;
  const DroneSpec spec_;
  DroneCommand command_{};

  std::atomic<bool> running_;
  std::thread worker_;

  mutable std::mutex tel_mtx_;
  mutable std::mutex command_mtx_;

  void step(float dt);

public:
  explicit DronePhysics(const IConfigLoader& c)
    : physics_time_step(c.get_config().physics_timestep)
    , timescale_(c.get_config().timescale)
    , telemetry_(c.get_config().position_, c.get_config().initial_direction_, c.get_config().altitude_)
    , spec_(c.get_config().attack_speed_, c.get_config().acceleration_path_, c.get_config().angular_speed_, c.get_config().turn_threshold_)
    , running_(true)
  {
  }

  void start(std::latch& latch);
  void interrupt();

  void submit_command(const DroneCommand& command) const override;

  const DroneTelemetry get_telemetry() const override;
  const DroneSpec get_spec() const override;
  const DroneCommand get_active_command() const override;
  const IState* get_state() const override;

  ~DronePhysics() override;
};