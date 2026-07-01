#pragma once

#include "DroneLink.hpp"
#include "ThreadSafeQueue.hpp"
#include "service/interfaces/ITargetProvider.hpp"

#include <atomic>
#include <chrono>
#include <latch>
#include <map>
#include <mutex>
#include <thread>

class UartTargetProvider : public ITargetProvider {
  struct TrackedTarget {
    Target target;
    std::chrono::steady_clock::time_point last_seen;
  };

  SynchronizedQueue<dlink::TargetPos>& target_channel_;

  const float scheduler_timestep_{};

  std::map<int, TrackedTarget> targets_{};

  std::atomic<bool> running_{false};
  std::thread worker_;

  mutable std::mutex mtx_;

  void update_target(const dlink::TargetPos& pos)
  {
    const auto now = std::chrono::steady_clock::now();
    const Coord coord{pos.x, pos.y};

    std::lock_guard<std::mutex> l(mtx_);

    auto it = targets_.find(pos.id);
    if (it == targets_.end()) {
      targets_.emplace(pos.id, TrackedTarget{Target{pos.id, coord, Coord{0.F, 0.F}}, now});
      return;
    }

    const float dt = std::chrono::duration<float>(now - it->second.last_seen).count();

    Coord velocity;

    if (dt > 0.F) {
      velocity = (coord - it->second.target.pos_) / dt;
    }
    else {
      velocity = Coord{0.F, 0.F};
    }

    it->second.target = Target{pos.id, coord, velocity};
    it->second.last_seen = now;
  }

public:
  explicit UartTargetProvider(SynchronizedQueue<dlink::TargetPos>& target_channel, float poll_period)
    : target_channel_(target_channel)
    , scheduler_timestep_(poll_period)
  {
  }

  const Target get_target(int id) const override
  {
    std::lock_guard<std::mutex> l(mtx_);
    return targets_.at(id).target;
  }

  std::vector<Target> get_targets() const override
  {
    std::lock_guard<std::mutex> l(mtx_);

    std::vector<Target> out;
    out.reserve(targets_.size());
    for (const auto& [id, tracked] : targets_) {
      out.push_back(tracked.target);
    }
    return out;
  }

  int get_size() const override
  {
    std::lock_guard<std::mutex> l(mtx_);
    return static_cast<int>(targets_.size());
  }

  void start(std::latch& latch)
  {
    running_ = true;

    worker_ = std::thread([this, &latch]() {
      latch.arrive_and_wait();

      using clock = std::chrono::steady_clock;
      const auto period = std::chrono::duration_cast<clock::duration>(std::chrono::duration<float>(scheduler_timestep_));

      auto next = clock::now() + period;
      while (running_) {
        for (const auto& pos : target_channel_.drain_all()) {
          update_target(pos);
        }
        std::this_thread::sleep_until(next);
        next += period;
      }
    });
  }

  void interrupt() { running_ = false; }

  ~UartTargetProvider() override
  {
    interrupt();
    if (worker_.joinable()) {
      worker_.join();
    }
  }
};
