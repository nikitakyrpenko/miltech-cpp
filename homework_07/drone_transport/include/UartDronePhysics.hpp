#pragma once

#include "DroneLink.hpp"
#include "ThreadSafeQueue.hpp"
#include "service/interfaces/IDronePhysics.hpp"

#include <atomic>
#include <chrono>
#include <latch>
#include <mutex>
#include <thread>

class UartDronePhysics : public IDronePhysics {
  SynchronizedQueue<dlink::Telemetry>& telemetry_channel_;

  // real-time gap between successive queue drains
  const float poll_period_{};

  DroneTelemetry telemetry_;
  const DroneSpec spec_;
  DroneCommand command_{};

  std::atomic<bool> running_{false};
  std::thread worker_;

  mutable std::mutex tel_mtx_;
  mutable std::mutex command_mtx_;

  void apply(const dlink::Telemetry& t)
  {
    std::lock_guard<std::mutex> l(tel_mtx_);
    telemetry_.set_position(Coord{t.x, t.y});
    telemetry_.set_current_speed(t.speed);
    telemetry_.set_current_direction(t.dir);
    telemetry_.set_elapsed(static_cast<float>(t.t_ms) / 1000.0F);
  }

public:
  UartDronePhysics(SynchronizedQueue<dlink::Telemetry>& telemetry_channel,
                   float poll_period,
                   const DroneSpec& spec,
                   const Coord& initial_position,
                   float initial_direction)
    : telemetry_channel_(telemetry_channel)
    , poll_period_(poll_period)
    , telemetry_(initial_position, initial_direction)
    , spec_(spec)
  {
  }

  // checker owns the real physics; just remember the intended command for get_active_command() parity
  void step(const DroneCommand& command, float /*dt*/) override
  {
    std::lock_guard<std::mutex> l(command_mtx_);
    command_ = command;
  }

  const DroneTelemetry get_telemetry() const override
  {
    std::lock_guard<std::mutex> l(tel_mtx_);
    return telemetry_;
  }

  const DroneSpec get_spec() const override { return spec_; }

  const DroneCommand get_active_command() const override
  {
    std::lock_guard<std::mutex> l(command_mtx_);
    return command_;
  }

  void start(std::latch& latch)
  {
    running_ = true;

    worker_ = std::thread([this, &latch]() {
      latch.arrive_and_wait();

      using clock = std::chrono::steady_clock;
      const auto period = std::chrono::duration_cast<clock::duration>(std::chrono::duration<float>(poll_period_));

      auto next = clock::now() + period;
      while (running_) {
        if (auto t = telemetry_channel_.drain_to_last()) {
          apply(*t);
        }
        std::this_thread::sleep_until(next);
        next += period;
      }
    });
  }

  void interrupt() { running_ = false; }

  ~UartDronePhysics() override
  {
    interrupt();
    if (worker_.joinable()) {
      worker_.join();
    }
  }
};
