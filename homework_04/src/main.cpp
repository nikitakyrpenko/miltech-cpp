#include <iostream>
#include <fstream>
#include <cmath>

int main(int argc, char** argv)
{
  if (argc != 2) {
    std::cerr << "usage: ugv_odometry <input_path>\n";

    return 1;
  }

  std::ifstream file(argv[argc - 1]);

  if (!file.is_open()) {
    std::cerr << "File : " << argv[argc - 1] << " not found" << std::endl;

    return 1;
  }

  int ticks_per_revolution{1024};
  float wheel_radius_m{0.3f};
  float wheelbase_m{1.0f};

  double x{0.0}, y{0.0}, theta{0.0};

  long prev_timestamp_ms;
  long prev_fl_ticks, prev_fr_ticks;
  long prev_bl_ticks, prev_br_ticks;

  file >> prev_timestamp_ms >> prev_fl_ticks >> prev_fr_ticks >> prev_bl_ticks >> prev_br_ticks;

  long next_timestamp_ms;
  long next_fl_ticks, next_fr_ticks;
  long next_bl_ticks, next_br_ticks;

  while (file >> next_timestamp_ms >> next_fl_ticks >> next_fr_ticks >> next_bl_ticks >> next_br_ticks) {
    long d_fl{std::abs(prev_fl_ticks - next_fl_ticks)};
    long d_fr{std::abs(prev_fr_ticks - next_fr_ticks)};
    long d_bl{std::abs(prev_bl_ticks - next_bl_ticks)};
    long d_br{std::abs(prev_br_ticks - next_br_ticks)};

    long d_left{(d_fl + d_bl) / 2};
    long d_right{(d_fr + d_br) / 2};

    double distance_per_tick{2.0 * M_PI * wheel_radius_m / ticks_per_revolution};

    double dL{d_left * distance_per_tick};
    double dR{d_right * distance_per_tick};

    double d{(dL + dR) / 2};
    double dtheta{(dR - dL) / wheelbase_m};

    x += d * std::cos(theta + dtheta / 2);
    y += d * std::sin(theta + dtheta / 2);
    theta += dtheta;

    std::cout << x << " " << y << " " << theta << std::endl;

    prev_timestamp_ms = next_timestamp_ms;
    prev_bl_ticks = next_bl_ticks;
    prev_br_ticks = next_br_ticks;
    prev_fl_ticks = next_fl_ticks;
    prev_fr_ticks = next_fr_ticks;
  }

  file.close();

  return 0;
}
