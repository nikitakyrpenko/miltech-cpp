#pragma once

#include "dto/TargetContext.hpp"
#include "service/interface/ITargetProvider.hpp"

class TargetProvider : public ITargetProvider {
  TargetContext* ctx_;

public:
  TargetProvider(TargetContext* ctx)
    : ctx_(ctx)
  {
  }

  Coord get_target(int target_id, float tick, float delta = 0.0F) const override;
  int get_size() const override { return ctx_->target_count_; }

  ~TargetProvider() override;
};
