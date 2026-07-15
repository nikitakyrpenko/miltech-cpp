#include "antidrone_turret/msg/servo_command.hpp"

#include <rclcpp/node.hpp>
#include <rclcpp/rclcpp.hpp>

namespace {
constexpr auto kServoTopic = "/servo/cmd";

using ServoType = antidrone_turret::msg::ServoCommand;
}  // namespace

class ServoDriverNode final : public rclcpp::Node {
public:
  ServoDriverNode()
    : rclcpp::Node("servo_driver_node")
  {
    subscription_ = this->create_subscription<ServoType>(kServoTopic, 10, [this](const ServoType& msg) { on_servo_message_received(msg); });
  }

private:
  auto on_servo_message_received(const ServoType& msg) -> void
  {
    const char* dir = msg.direction > 0 ? "RIGHT" : msg.direction < 0 ? "LEFT" : "CENTER";
    RCLCPP_INFO(get_logger(), "servo received dir=%s target_y=%.1f error_y=%.2f", dir, msg.target_x, msg.error_x);
  };

  rclcpp::Subscription<ServoType>::SharedPtr subscription_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ServoDriverNode>());
  rclcpp::shutdown();
  return 0;
}
