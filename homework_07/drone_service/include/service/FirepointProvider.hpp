#include "service/interfaces/IFirepointProvider.hpp"

class FirepointProvider : public IFirepointProvider {
public:
  const BallisticSolution solve(const Drone& drone, const FallResult& fall, const Coord& target) const override;

  ~FirepointProvider() override;
};