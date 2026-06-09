#include "vehicle.hpp"
#include "drone_exceptions.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

Vehicle::Vehicle(const std::string& name, float battery_level)
    : name(name), battery_level(battery_level), status("idle") {
    if (this->battery_level < 0.0f) this->battery_level = 0.0f;
    if (this->battery_level > 100.0f) this->battery_level = 100.0f;
}

void Vehicle::drain_battery(float amount) {
    if (battery_level <= 0.0f) {
        throw BatteryDepletedError("Drone '" + name + "': Critical failure! Battery is completely depleted.");
    }
    battery_level -= amount;
    if (battery_level < 0.0f) battery_level = 0.0f;
}

void Vehicle::charge_battery(float amount, int duration_seconds) {
    if (status != "charging") {
        throw InvalidStateError("Drone '" + name + "': Cannot charge battery when status is '" + status + "'. Must be 'charging'.");
    }
    battery_level += amount;
    if (battery_level > 100.0f) battery_level = 100.0f;
    add_log_entry("Charged battery by " + std::to_string(amount) + "% over " + std::to_string(duration_seconds) + " seconds.");
}

bool Vehicle::is_critical() const {
    return battery_level < 20.0f;
}

void Vehicle::set_status(const std::string& new_status) {
    if (new_status != "idle" && new_status != "flying" && new_status != "charging") {
        throw InvalidStateError("Drone '" + name + "': Mode '" + new_status + "' is not a valid operating state.");
    }
    status = new_status;
    add_log_entry("Status systematically changed to: " + status);
}

void Vehicle::add_log_entry(const std::string& entry) {
    flight_log.push_back("[" + get_current_timestamp() + "] " + entry);
}

std::string Vehicle::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

std::string Vehicle::get_flight_log() const {
    std::string summary;
    for (const auto& log : flight_log) {
        summary += log + "\n";
    }
    return summary;
}

std::string Vehicle::get_name() const { return name; }
float Vehicle::get_battery_level() const { return battery_level; }
std::string Vehicle::get_status() const { return status; }
