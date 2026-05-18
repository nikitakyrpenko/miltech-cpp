#include "dto.hpp"

const static float G = 9.81F;

float calcTAmmo(const Drone& drone, const Ammo& ammo);
float calcHDist(const Drone& drone, const Ammo& ammo, float tAmmo);

Result calcFirePosition(const Coord& position, const Coord& target, FirePosition& outFirePosition);
