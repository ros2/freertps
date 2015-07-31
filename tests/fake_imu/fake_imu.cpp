#include <cstdio>
#include <rclcpp/rclcpp.hpp>
#include <sensor_interfaces/msg/imu.hpp>

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("fake_imu");
  auto pub = node->create_publisher<sensor_interfaces::msg::Imu>("imu", 1);
  rclcpp::WallRate loop_rate(2);
  auto msg = std::make_shared<sensor_interfaces::msg::Imu>();
  printf("greetings, i will now start publishing fake IMU data\n");
  msg->header.stamp.sec = 1234;
  msg->header.stamp.nanosec = 5678;
  msg->header.frame_id = "imu_frame";
  int pub_count = 0;
  while (rclcpp::ok())
  {
    pub_count++;
    msg->orientation.x = 1 + pub_count;
    msg->orientation.y = 2;
    msg->orientation.z = 3;
    msg->orientation.w = 4;
    msg->angular_velocity.x = 5;
    msg->angular_velocity.y = 6;
    msg->angular_velocity.z = 7;
    msg->linear_acceleration.x = 8;
    msg->linear_acceleration.y = 9;
    msg->linear_acceleration.z = 10;
    for (int i = 0; i < 9; i++)
    {
      msg->orientation_covariance[i] = i + 11;
      msg->angular_velocity_covariance[i] = i + 20;
      msg->linear_acceleration_covariance[i] = i + 29;
    }
    printf("pub\n");
    pub->publish(msg);
    rclcpp::spin_some(node);
    loop_rate.sleep();
  }
  return 0;
}
