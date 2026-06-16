#pragma once

#include "service/interfaces/ITargetProvider.hpp"
class TargetProviderIterator {
private:
  const ITargetProvider* provider_;
  int index_;

public:
  TargetProviderIterator(const ITargetProvider* provider, int index)
    : provider_(provider)
    , index_(index)
  {
  }

  inline const Target& get() { return provider_->get_target(index_); }
  inline bool has_next() { return index_ < provider_->size(); }
  inline TargetProviderIterator& next()
  {
    index_++;
    return *this;
  }
};