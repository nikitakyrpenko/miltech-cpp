#pragma once

#include "service/interfaces/IMissionProcessor.hpp"

#include <memory>

enum class LoaderType { JSON };
enum class SolverType { ANALYTICAL, TABLE };

class IMissionFactory {
public:
  virtual std::unique_ptr<IMissionProccessor> create(LoaderType loader_type,
                                                     SolverType solver_type,
                                                     const char* config_source,
                                                     const char* ammo_source,
                                                     const char* target_source,
                                                     const char* table_source = nullptr) const = 0;

  virtual ~IMissionFactory() = default;
};
