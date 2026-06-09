cmake_minimum_required(VERSION 3.8)
project(drone_fleet)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)

include_directories(include ../../../part1/include)

add_library(drone_core SHARED
  ../../../part1/src/vehicle.cpp
  ../../../part1/src/drone.cpp
  ../../../part1/src/mission_drone.cpp
  ../../../part1/src/autonomous_drone.cpp
)

add_executable(drone_node src/drone_node.cpp)
target_link_libraries(drone_node drone_core)
ament_target_dependencies(drone_node rclcpp std_msgs)

add_executable(fleet_manager src/fleet_manager.cpp)
target_link_libraries(fleet_manager drone_core)
ament_target_dependencies(fleet_manager rclcpp std_msgs std_srvs)

add_executable(health_monitor src/health_monitor.cpp)
target_link_libraries(health_monitor drone_core)
ament_target_dependencies(health_monitor rclcpp std_msgs)

install(TARGETS drone_node fleet_manager health_monitor
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
