#include "mission_drone.hpp"
#include <sstream>

MissionDrone::MissionDrone(const std::string& name, float battery_level, float max_altitude, float speed,
                           const std::string& mission_name, const std::vector<std::tuple<float, float, float>>& waypoints)
    : Drone(name, battery_level, max_altitude, speed), mission_name(mission_name), waypoints(waypoints), current_waypoint_index(0) {}

std::string MissionDrone::get_info() const {
    return "[MissionDrone] Name: " + get_name() + " | Mission: " + mission_name + 
           " | WP: " + std::to_string(current_waypoint_index) + "/" + std::to_string(waypoints.size()) +
           " | Battery: " + std::to_string(get_battery_level()) + "%";
}

std::tuple<float, float, float> MissionDrone::next_waypoint() {
    if (mission_complete()) {
        return waypoints.back();
    }
    auto wp = waypoints[current_waypoint_index];
    drain_battery(1.5f);
    visited_waypoints.push_back({wp, get_current_timestamp()});
    add_log_entry("Reached Waypoint " + std::to_string(current_waypoint_index) + ": (" +
                  std::to_string(std::get<0>(wp)) + ", " + std::to_string(std::get<1>(wp)) + ", " + std::to_string(std::get<2>(wp)) + ")");
    current_waypoint_index++;
    return wp;
}

void MissionDrone::skip_waypoint(const std::string& reason) {
    if (mission_complete()) return;
    add_log_entry("Skipped Waypoint " + std::to_string(current_waypoint_index) + ". Reason: " + reason);
    current_waypoint_index++;
}

bool MissionDrone::mission_complete() const {
    return current_waypoint_index >= static_cast<int>(waypoints.size());
}

std::string MissionDrone::mission_summary() const {
    std::stringstream ss;
    ss << "=== Mission Summary for [" << mission_name << "] ===\n";
    ss << "Drone: " << get_name() << "\nTotal Waypoints: " << waypoints.size() << "\nVisited Route:\n";
    for (size_t i = 0; i < visited_waypoints.size(); ++i) {
        auto wp = visited_waypoints[i].first;
        ss << "  - [" << visited_waypoints[i].second << "] WP " << i << ": ("
           << std::get<0>(wp) << ", " << std::get<1>(wp) << ", " << std::get<2>(wp) << ")\n";
    }
    return ss.str();
}

std::string MissionDrone::get_mission_name() const { return mission_name; }
int MissionDrone::get_current_waypoint_index() const { return current_waypoint_index; }
size_t MissionDrone::get_total_waypoints() const { return waypoints.size(); }
