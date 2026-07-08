#pragma once

#include "DroneLink.hpp"
#include "ScheduledWorker.hpp"
#include "UartLink.hpp"
#include "service/interfaces/ITargetProvider.hpp"

#include <chrono>
#include <map>
#include <memory>
#include <mutex>

class UartTargetProvider : public ITargetProvider, public ScheduledWorker {
  std::shared_ptr<UartLink> link_;
  SynchronizedQueue<dlink::TargetPos>& target_channel_;
  std::map<int, Target> targets_{};
  mutable std::mutex mtx_;

  void update_target(const dlink::TargetPos& pos);
  void tick() override;

public:
  explicit UartTargetProvider(std::shared_ptr<UartLink> link, float idle);

  const Target get_target(int id) const override;
  std::vector<Target> get_targets() const override;
  int get_size() const override;
};
