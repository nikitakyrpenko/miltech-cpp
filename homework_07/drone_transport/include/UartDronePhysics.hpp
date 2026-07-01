#pragma once

#include "DroneLink.hpp"
#include "ScheduledWorker.hpp"
#include "ThreadSafeQueue.hpp"
#include "UartLink.hpp"
#include "service/interfaces/IConfigLoader.hpp"
#include "service/interfaces/IDronePhysics.hpp"

#include <memory>
#include <mutex>
#include <optional>

class UartDronePhysics : public IDronePhysics, public ScheduledWorker {
  std::shared_ptr<UartLink> link_;
  SynchronizedQueue<dlink::Telemetry>& telemetry_channel_;
  std::shared_ptr<SynchronizedQueue<DroneCommand>> command_channel_;

  std::optional<DroneTelemetry> telemetry_;
  const DroneSpec spec_;
  DroneCommand command_{};

  mutable std::mutex tel_mtx_;
  mutable std::mutex command_mtx_;

  static const IState* state_from_mode(DroneMode mode);
  void apply(const dlink::Telemetry& t);
  dlink::Control to_control(const DroneCommand& cmd) const;
  void tick() override;

public:
  UartDronePhysics(std::shared_ptr<UartLink> link,
                   const IConfigLoader& config_loader,
                   std::shared_ptr<SynchronizedQueue<DroneCommand>> command_channel,
                   float poll_period);

  bool is_ready() const;

  void step(const DroneCommand& command, float dt) override;
  const DroneTelemetry get_telemetry() const override;
  const DroneSpec get_spec() const override;
  const DroneCommand get_active_command() const override;
  const IState* get_state() const override;
};
