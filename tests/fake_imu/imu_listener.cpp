#include <cstdio>
#include <rclcpp/rclcpp.hpp>
#include <sensor_interfaces/msg/imu.hpp>

void imu_cb(const sensor_interfaces::msg::Imu::ConstSharedPtr &msg)
{
  printf(" accel: [%+6.3f %+6.3f %+6.3f]\n",
         msg->linear_acceleration.x,
         msg->linear_acceleration.y,
         msg->linear_acceleration.z);
}

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("imu_listener");
  auto sub = node->create_subscription<sensor_interfaces::msg::Imu>
               ("imu", 1, imu_cb);
  rclcpp::spin(node);
  return 0;
}
