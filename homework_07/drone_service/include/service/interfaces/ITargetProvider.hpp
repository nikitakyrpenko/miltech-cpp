#pragma once

#include "models/Target.hpp"

class TargetProviderIterator;

class ITargetProvider {
public:
  virtual const Target& get_target(int id) const = 0;
  virtual int size() const = 0;

  virtual TargetProviderIterator begin() = 0;
  virtual TargetProviderIterator end() = 0;

  virtual ~ITargetProvider() = default;
};