#pragma once

#include "DroneLink.hpp"
#include "ThreadSafeQueue.hpp"
#include "ThreadWorker.hpp"
#include "UartPort.hpp"

#include <array>
#include <memory>

class UartLink : public ThreadWorker {
  std::unique_ptr<UartPort> port_;

  SynchronizedQueue<dlink::Telemetry> telemetry_channel_;
  SynchronizedQueue<dlink::TargetPos> target_channel_;
  SynchronizedQueue<dlink::AmmoCfg> ammo_channel_;
  SynchronizedQueue<dlink::Result> result_channel_;
  SynchronizedQueue<dlink::DroneCfg> config_channel_;

  // debug aid: prints once the first time each distinct packet type is seen on the wire
  std::array<bool, 256> seen_packet_types_{};

  void run_loop() override;

public:
  explicit UartLink(const char* serial);

  SynchronizedQueue<dlink::Telemetry>& telemetry_channel() { return telemetry_channel_; }
  SynchronizedQueue<dlink::TargetPos>& target_channel() { return target_channel_; }
  SynchronizedQueue<dlink::AmmoCfg>& ammo_channel() { return ammo_channel_; }
  SynchronizedQueue<dlink::Result>& result_channel() { return result_channel_; }
  SynchronizedQueue<dlink::DroneCfg>& config_channel() { return config_channel_; }

  void send(const dlink::Control& ctrl_frame);
};
