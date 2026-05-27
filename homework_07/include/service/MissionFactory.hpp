#pragma once

#include "service/interface/IBallisticSolver.hpp"
#include "service/interface/IConfigLoader.hpp"
#include "service/interface/IConfigProvider.hpp"
#include "service/interface/ITargetProvider.hpp"
#include "service/MissionProccessor.hpp"

enum class LoaderType { JSON };
enum class SolverType { ANALYTICAL };

struct Mission {
  IConfigLoader* loader;
  IConfigProvider* config;
  ITargetProvider* targets;
  IBallisticSolver* solver;
  MissionProccessor* processor;

  ~Mission()
  {
    delete processor;
    delete solver;
    delete config;
    delete targets;
    delete loader;
  }
};

class MissionFactory {
public:
  static Mission* create(
    LoaderType loader_type, SolverType solver_type, const char* sim_file, const char* ammo_file, const char* target_file);
};
