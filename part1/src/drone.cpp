#include "drone.hpp"
#include "drone_exceptions.hpp"

Drone::Drone(const std::string& name, float battery_level, float max_altitude, float speed)
    : Vehicle(name, battery_level), altitude(0.0f), max_altitude(max_altitude), speed(speed) {}

std::string Drone::get_info() const {
    return "[Drone] Name: " + get_name() + " | Battery: " + std::to_string(get_battery_level()) +
           "% | Status: " + get_status() + " | Altitude: " + std::to_string(altitude) + "m";
}

void Drone::take_off(float target_altitude) {
    if (target_altitude > max_altitude) {
        throw AltitudeError("Drone '" + get_name() + "': Flight target (" + std::to_string(target_altitude) + 
                            "m) exceeds design ceiling of " + std::to_string(max_altitude) + "m.");
    }
    set_status("flying");
    altitude = target_altitude;
    add_log_entry("Successful takeoff performed to altitude: " + std::to_string(altitude) + "m.");
}

void Drone::land() {
    altitude = 0.0f;
    set_status("idle");
    add_log_entry("Autonomous landing sequence executed successfully.");
}

void Drone::emergency_stop() {
    add_log_entry("CRITICAL: EMERGENCY STOP INVOKED.");
    drain_battery(30.0f);
    altitude = 0.0f;
    set_status("idle");
}

float Drone::get_altitude() const { return altitude; }
float Drone::get_max_altitude() const { return max_altitude; }
float Drone::get_speed() const { return speed; }
