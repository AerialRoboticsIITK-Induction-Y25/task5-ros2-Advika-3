#ifndef MISSION_DRONE_HPP
#define MISSION_DRONE_HPP

#include "drone.hpp"
#include <vector>
#include <tuple>
#include <utility>

class MissionDrone : public Drone {
public:
    MissionDrone(const std::string& name, float battery_level, float max_altitude, float speed, 
                 const std::string& mission_name, const std::vector<std::tuple<float, float, float>>& waypoints);
    virtual ~MissionDrone() = default;

    std::string get_info() const override;

    std::tuple<float, float, float> next_waypoint();
    void skip_waypoint(const std::string& reason);
    bool mission_complete() const;
    std::string mission_summary() const;

    std::string get_mission_name() const;
    int get_current_waypoint_index() const;
    size_t get_total_waypoints() const;

protected:
    std::string mission_name;
    std::vector<std::tuple<float, float, float>> waypoints;
    int current_waypoint_index;

private:
    std::vector<std::pair<std::tuple<float, float, float>, std::string>> visited_waypoints;
};

#endif // MISSION_DRONE_HPP
