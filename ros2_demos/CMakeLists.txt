cmake_minimum_required(VERSION 2.8.3)

project(freertps_ros2_demos)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rmw_implementation REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

#find_package(OpenCV REQUIRED)

include_directories(include)

add_executable_for_each_rmw_implementations(led_blink
  led_blink.cpp
  TARGET_DEPENDENCIES
  "rclcpp"
  "sensor_msgs"
  "std_msgs"
  #"OpenCV"
  INSTALL
)
ament_package()
