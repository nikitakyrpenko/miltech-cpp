#pragma once

#include "service/interfaces/IConfigLoader.hpp"
#include "service/interfaces/ITargetProvider.hpp"

#include <atomic>
#include <latch>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class TargetProvider : public ITargetProvider {
  const std::shared_ptr<IConfigLoader> config_;

  // real-time spacing between the precomputed target positions (indexes the array + derives velocity)
  const float array_timestep_{};
  // how often the worker thread recomputes the target snapshot
  const float scheduler_timestep_{};
  // wall-clock speed-up factor (simulated_time / real_time)
  const float timescale_{};

  std::vector<Target> targets_{};

  const int number_of_targets_{};
  const int number_of_positions_per_target_{};

  std::atomic<bool> running_{false};
  std::thread worker_;

  mutable std::mutex mtx_;

  void update_targets(const std::chrono::time_point<std::chrono::steady_clock>& start_,
                      const std::chrono::time_point<std::chrono::steady_clock>& now_);

public:
  TargetProvider(const std::shared_ptr<IConfigLoader> config)
    : config_(std::move(config))
    , array_timestep_(config->get_config().target_array_timestep_)
    , scheduler_timestep_(config->get_config().target_timestep)
    , timescale_(config->get_config().timescale)
    , number_of_targets_(config->get_targets().target_count_)
    , number_of_positions_per_target_(config->get_targets().time_steps_)
  {
  }

  const Target get_target(int i) const override;
  std::vector<Target> get_targets() const override;
  int get_size() const override;

  void start(std::latch& latch);
  void interrupt();

  ~TargetProvider() override;
};