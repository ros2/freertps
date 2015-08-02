cmake_minimum_required(VERSION 2.8.3)

project(fake_imu)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
endif()

find_package(ament_cmake REQUIRED)
find_package(sensor_interfaces REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rmw REQUIRED)
find_package(rmw_implementation REQUIRED)
find_package(std_interfaces REQUIRED)

ament_package()

# This file provides the add_executable_for_each_rmw_impl function.
include(cmake/add_executable_for_each_rmw_impl.cmake)

add_executable_for_each_rmw_impl(fake_imu fake_imu.cpp)
add_executable_for_each_rmw_impl(imu_listener imu_listener.cpp)

