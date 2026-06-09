#include "autonomous_drone.hpp"
#include <cmath>

AutonomousDrone::AutonomousDrone(const std::string& name, float battery_level, float max_altitude, float speed,
                                 const std::string& mission_name, const std::vector<std::tuple<float, float, float>>& waypoints,
                                 std::tuple<float, float, float> home_position)
    : MissionDrone(name, battery_level, max_altitude, speed, mission_name, waypoints), ai_mode("auto"), home_position(home_position) {}

std::string AutonomousDrone::get_info() const {
    return "[AutonomousDrone] Name: " + get_name() + " | AI Mode: " + ai_mode + " | Battery: " + std::to_string(get_battery_level()) + "%";
}

void AutonomousDrone::set_ai_mode(const std::string& mode) {
    ai_mode = mode;
    add_log_entry("AI guidance configuration shifted to: " + ai_mode);
    if (ai_mode == "return_home") {
        add_log_entry("RTL Protocol standard: Injecting Home coordinates into path planning.");
        waypoints.insert(waypoints.begin() + current_waypoint_index, home_position);
    }
}

void AutonomousDrone::detect_obstacle(std::tuple<float, float, float> position, const std::string& severity) {
    std::string log_msg = "[" + get_current_timestamp() + "] Obstacle discovered at (" +
                          std::to_string(std::get<0>(position)) + ", " + std::to_string(std::get<1>(position)) + ", " + std::to_string(std::get<2>(position)) + 
                          ") | Severity: " + severity;
    obstacle_log.push_back(log_msg);
    add_log_entry("Sensing Array alert: Threat detection tier - " + severity);

    if (severity == "high") {
        emergency_stop();
    }
}

std::vector<std::tuple<float, float, float>> AutonomousDrone::auto_replan(const std::vector<std::tuple<float, float, float>>& obstacles) {
    std::vector<std::tuple<float, float, float>> clear_waypoints;
    for (const auto& wp : waypoints) {
        bool unsafe = false;
        for (const auto& obs : obstacles) {
            float dx = std::get<0>(wp) - std::get<0>(obs);
            float dy = std::get<1>(wp) - std::get<1>(obs);
            float dz = std::get<2>(wp) - std::get<2>(obs);
            float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (distance < 5.0f) {
                unsafe = true;
                break;
            }
        }
        if (!unsafe) {
            clear_waypoints.push_back(wp);
        } else {
            // Replan offset safely
            clear_waypoints.push_back({std::get<0>(wp) + 6.0f, std::get<1>(wp) + 6.0f, std::get<2>(wp)});
        }
    }
    waypoints = clear_waypoints;
    add_log_entry("Path planning updated locally to avert collision criteria vector bounds.");
    return waypoints;
}

std::string AutonomousDrone::get_ai_mode() const { return ai_mode; }
