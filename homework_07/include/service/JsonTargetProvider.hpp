#pragma once

#include "dto/TargetContext.hpp"
#include "service/interface/ITargetProvider.hpp"

class JsonTargetProvider : public ITargetProvider {
  TargetContext* ctx_;

public:
  JsonTargetProvider(TargetContext* ctx)
    : ctx_(ctx)
  {
  }
  ~JsonTargetProvider() override { delete ctx_; }

  Coord get_target(int target_id, float tick, float delta = 0.0F) const override;
  int get_size() const override { return ctx_->target_count_; }
};
