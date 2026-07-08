struct NEDPosition {
  float north_m;
  float east_m;

  bool operator==(const NEDPosition& other) const
  {
    if (this->north_m == other.north_m && this->east_m == other.east_m) {
      return true;
    }
    return false;
  }
};