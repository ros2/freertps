// Copyright 2015 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::node::Node::make_shared("led_blink");
  rmw_qos_profile_t qos;
  qos.reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
  qos.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  qos.depth = 1;
  auto pub = node->create_publisher<std_msgs::msg::Bool>("led", qos);
  auto msg = std::make_shared<std_msgs::msg::Bool>();
  rclcpp::WallRate loop_rate(4);
  while (rclcpp::ok())
  {
    msg->data = !msg->data;
    pub->publish(msg);
    rclcpp::spin_some(node);
    loop_rate.sleep();
  }
  return 0;
}
