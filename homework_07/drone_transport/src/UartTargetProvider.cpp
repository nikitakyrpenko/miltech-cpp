#include "UartTargetProvider.hpp"
#include "UartLink.hpp"

UartTargetProvider::UartTargetProvider(std::shared_ptr<UartLink> link, float idle)
  : ScheduledWorker(idle)
  , link_(std::move(link))
  , target_channel_(link_->target_channel())
{
}

void UartTargetProvider::update_target(const dlink::TargetPos& pos)
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

  const Coord velocity = (dt > 0.F) ? (coord - it->second.target.pos_) / dt : Coord{0.F, 0.F};

  it->second.target = Target{pos.id, coord, velocity};
  it->second.last_seen = now;
}

const Target UartTargetProvider::get_target(int id) const
{
  std::lock_guard<std::mutex> l(mtx_);
  return targets_.at(id).target;
}

std::vector<Target> UartTargetProvider::get_targets() const
{
  std::lock_guard<std::mutex> l(mtx_);

  std::vector<Target> out;
  out.reserve(targets_.size());
  for (const auto& [id, tracked] : targets_) {
    out.push_back(tracked.target);
  }
  return out;
}

int UartTargetProvider::get_size() const
{
  std::lock_guard<std::mutex> l(mtx_);
  return static_cast<int>(targets_.size());
}

void UartTargetProvider::tick()
{
  for (const auto& pos : target_channel_.drain_all()) {
    update_target(pos);
  }
}
