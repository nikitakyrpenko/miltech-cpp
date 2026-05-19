enum Result { OK, FileNotFound, FileParsingError, UnknownAmmo, BadAltitude, BadAttackSpeed, BadAccelerationPath, DroneToHigh };

struct Coord {
  float x, y, z;
};

struct Drone {
  Coord position;
  float at;
  float ap;
};

struct Ammo {
  char name[32];
  float mass;
  float drag;
  float lift;
};

struct FirePosition {
  Coord intermidiate;
  Coord fire;
  bool hasIntermidiate;
};