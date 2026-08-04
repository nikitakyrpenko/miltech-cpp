#pragma once

#include "underground_world/scenario.hpp"
#include "underground_world/world_model.hpp"

#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>

class PathfinderService {
  struct Distance {
    underground_world::Position parent;
    int steps = 0;
  };

  underground_world::WorldMap map_{};

  std::optional<underground_world::Position> pathfinder_;
  std::optional<underground_world::Move> direction_;

  const std::vector<const underground_world::ObservedCell*> get_frontiers() const;
  std::unordered_map<underground_world::Position, Distance, underground_world::Position::Hash> distances_from(
    const underground_world::Position& start) const;

  const std::optional<const underground_world::ObservedCell*> rank_frontiers(
    const std::unordered_map<underground_world::Position, Distance, underground_world::Position::Hash>& distances,
    std::vector<const underground_world::ObservedCell*> frontiers) const;

  std::queue<const underground_world::ObservedCell*> route_to_frontier() const;
  std::optional<underground_world::ObservedCell> enemy_scan() const;

public:
  struct Step {
    underground_world::Status status = underground_world::Status::EXPLORING;
    underground_world::Move move = underground_world::Move::NONE;
    std::optional<underground_world::ObservedCell> enemy_at;
  };

  void chart(const underground_world::LocalScanData& data);
  Step derive_step() const;
};