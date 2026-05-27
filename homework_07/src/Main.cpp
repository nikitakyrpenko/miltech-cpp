#include "service/MissionFactory.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <config.json> <ammo.json> <targets.json>" << std::endl;
    return 1;
  }

  Mission* m = MissionFactory::create(LoaderType::JSON, SolverType::ANALYTICAL, argv[1], argv[2], argv[3]);

  if (!m)
    return 1;

  auto steps = m->processor->run();

  delete m;
  return 0;
}
