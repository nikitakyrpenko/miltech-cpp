
#include "underground_world/world_model.hpp"

#include "underground_world/pathfinder_service.hpp"
#include <array>
#include <optional>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace {

constexpr std::array<underground_world::Position, 4> neighbor_deltas{
  underground_world::Position{-1, 0},
  underground_world::Position{1, 0},
  underground_world::Position{0, -1},
  underground_world::Position{0, 1},
};

underground_world::Move move_between(const underground_world::Position& from, const underground_world::Position& to)
{
  const int dx = to.x - from.x;
  const int dy = to.y - from.y;
  if (dx == 1 && dy == 0) {
    return underground_world::Move::RIGHT;
  }
  if (dx == -1 && dy == 0) {
    return underground_world::Move::LEFT;
  }
  if (dx == 0 && dy == 1) {
    return underground_world::Move::DOWN;
  }
  if (dx == 0 && dy == -1) {
    return underground_world::Move::UP;
  }
  return underground_world::Move::NONE;
}

}  // namespace

void PathfinderService::chart(const underground_world::LocalScanData& data)
{
  map_.chart(data);
  pathfinder_ = data.robot;
}

const std::vector<const underground_world::ObservedCell*> PathfinderService::get_frontiers() const
{
  std::vector<const underground_world::ObservedCell*> frontiers{};

  for (const auto p : map_.get_all_passable()) {
    if (!map_.does_position_neighbors_charted(p->position)) {
      frontiers.push_back(p);
    }
  }
  return frontiers;
}

std::unordered_map<underground_world::Position, PathfinderService::Distance, underground_world::Position::Hash>
PathfinderService::distances_from(const underground_world::Position& start) const
{
  const auto& map = map_.get_map();

  std::unordered_map<underground_world::Position, Distance, underground_world::Position::Hash> distances{};
  std::queue<underground_world::Position> queue{};

  distances.emplace(start, Distance{start, 0});
  queue.push(start);

  while (!queue.empty()) {
    const auto current = queue.front();
    queue.pop();
    const auto current_steps = distances.at(current).steps;

    for (const auto& delta : neighbor_deltas) {
      const underground_world::Position next{current.x + delta.x, current.y + delta.y};
      if (distances.contains(next)) {
        continue;
      }
      const auto cell = map.find(next);
      if (cell == map.end() || cell->second.kind == underground_world::CellKind::Wall) {
        continue;
      }
      distances.emplace(next, Distance{current, current_steps + 1});
      queue.push(next);
    }
  }

  return distances;
}

const std::optional<const underground_world::ObservedCell*> PathfinderService::rank_frontiers(
  const std::unordered_map<underground_world::Position, Distance, underground_world::Position::Hash>& distances,
  std::vector<const underground_world::ObservedCell*> frontiers) const
{
  if (frontiers.empty()) {
    return std::nullopt;
  }

  const underground_world::ObservedCell* best = nullptr;
  int best_steps = 0;

  for (const auto* frontier : frontiers) {
    const auto distance = distances.find(frontier->position);
    if (distance == distances.end()) {
      continue;
    }

    const int steps = distance->second.steps;
    if (best == nullptr || steps < best_steps ||
        (steps == best_steps && std::tie(frontier->position.x, frontier->position.y) < std::tie(best->position.x, best->position.y))) {
      best = frontier;
      best_steps = steps;
    }
  }

  if (best == nullptr) {
    return std::nullopt;
  }
  return best;
}

std::queue<const underground_world::ObservedCell*> PathfinderService::route_to_frontier() const
{
  std::queue<const underground_world::ObservedCell*> route{};

  if (!pathfinder_) {
    return route;
  }

  const auto distances = distances_from(*pathfinder_);
  const auto target = rank_frontiers(distances, get_frontiers());
  if (!target) {
    return route;
  }

  const auto& map = map_.get_map();

  std::vector<const underground_world::ObservedCell*> reversed{};
  underground_world::Position current = (*target)->position;

  while (!(current == *pathfinder_)) {
    const auto step = distances.find(current);
    const auto cell = map.find(current);
    if (step == distances.end() || cell == map.end()) {
      return {};
    }
    reversed.push_back(&cell->second);
    current = step->second.parent;
  }

  for (auto cell = reversed.rbegin(); cell != reversed.rend(); ++cell) {
    route.push(*cell);
  }

  return route;
}

std::optional<underground_world::ObservedCell> PathfinderService::enemy_scan() const
{
  for (const auto* cell : map_.get_all_passable()) {
    if (cell->kind == underground_world::CellKind::Contact) {
      return *cell;
    }
  }
  return std::nullopt;
}

PathfinderService::Step PathfinderService::derive_step() const
{
  if (!pathfinder_) {
    return Step{underground_world::Status::EXPLORING, underground_world::Move::NONE, std::nullopt};
  }

  if (const auto enemy = enemy_scan()) {
    return Step{underground_world::Status::ENGAGING, underground_world::Move::NONE, enemy};
  }

  auto route = route_to_frontier();

  if (route.empty()) {
    return Step{underground_world::Status::DONE, underground_world::Move::NONE, std::nullopt};
  }

  return Step{underground_world::Status::EXPLORING, move_between(*pathfinder_, route.front()->position), std::nullopt};
}
