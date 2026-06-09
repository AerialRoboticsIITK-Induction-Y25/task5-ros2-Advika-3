#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"
#include <string>
#include <unordered_map>
#include <iostream>
#include <iomanip>

struct DroneReportState {
    std::string name = "Unknown";
    float battery = 0.0f;
    float altitude = 0.0f;
    std::string waypoint = "0/0";
    std::string status = "offline";
};

class FleetManager : public rclcpp::Node {
public:
    FleetManager() : Node("fleet_manager") {
        std::vector<std::string> target_drones = {"Alpha", "Beta", "Gamma"};

        for (const auto& name : target_drones) {
            fleet_store_[name] = DroneReportState{name, 0.0f, 0.0f, "0/5", "offline"};

            // Status Subscriptions
            status_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/status", 10,
                [this, name](const std_msgs::msg::String::SharedPtr msg) { this->handle_status(name, msg->data); }));

            // Alert Subscriptions
            alert_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/alert", 10,
                [this, name](const std_msgs::msg::String::SharedPtr msg) { this->handle_alert(name, msg->data); }));

            // Mission Complete Subscriptions
            complete_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/mission_complete", 10,
                [this, name](const std_msgs::msg::String::SharedPtr msg) { 
                    RCLCPP_INFO(this->get_logger(), "[FLEET EVENT] Drone %s reports complete mission path execution.", name.c_str()); 
                }));

            // Telemetry Subscriptions
            telemetry_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/telemetry", 10,
                [this, name](const std_msgs::msg::String::SharedPtr msg) { this->handle_telemetry_json(name, msg->data); }));
        }

        report_timer_ = this->create_wall_timer(std::chrono::seconds(5), std::bind(&FleetManager::generate_table_report, this));
        
        report_service_ = this->create_service<std_srvs::srv::Trigger>(
            "/fleet/status_report",
            std::bind(&FleetManager::trigger_report_service, this, std::placeholders::_1, std::placeholders::_2));
    }

private:
    void handle_status(const std::string& name, const std::string& data) {
        // String split based manual parsing logic for status format
        size_t pos = 0;
        std::string token;
        std::string s = data;
        while ((pos = s.find("|")) != std::string::npos || !s.empty()) {
            if (pos != std::string::npos) {
                token = s.substr(0, pos);
                s.erase(0, pos + 1);
            } else {
                token = s;
                s.clear();
            }
            
            size_t delim = token.find(":");
            if (delim != std::string::npos) {
                std::string key = token.substr(0, delim);
                std::string val = token.substr(delim + 1);
                if (key == "waypoint") fleet_store_[name].waypoint = val;
            }
        }
    }

    void handle_telemetry_json(const std::string& name, const std::string& raw_json) {
        // Safe robust zero-dependency substring extraction logic for JSON elements
        fleet_store_[name].battery = std::stof(extract_json_field(raw_json, "battery"));
        fleet_store_[name].altitude = std::stof(extract_json_field(raw_json, "altitude"));
        fleet_store_[name].status = extract_json_field(raw_json, "status");
    }

    void handle_alert(const std::string& name, const std::string& data) {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char timestamp[30];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %X", std::localtime(&now));
        RCLCPP_WARN(this->get_logger(), "[ALERT LOG][%s] Drone: %s -> %s", timestamp, name.c_str(), data.c_str());
    }

    std::string extract_json_field(const std::string& json, const std::string& key) {
        size_t key_pos = json.find("\"" + key + "\"");
        if (key_pos == std::string::npos) return "0";
        size_t start_pos = json.find(":", key_pos);
        if (start_pos == std::string::npos) return "0";
        start_pos++;
        
        while(start_pos < json.size() && (json[start_pos] == ' ' || json[start_pos] == '"')) {
            start_pos++;
        }
        
        size_t end_pos = start_pos;
        while(end_pos < json.size() && json[end_pos] != ',' && json[end_pos] != '}' && json[end_pos] != '"') {
            end_pos++;
        }
        return json.substr(start_pos, end_pos - start_pos);
    }

    void generate_table_report() {
        std::cout << "\n=========================================================\n";
        std::cout << "               CENTRAL FLEET MANAGER REPORT              \n";
        std::cout << "=========================================================\n";
        std::cout << std::left << std::setw(12) << "Drone" 
                  << std::setw(12) << "Battery (%)" 
                  << std::setw(15) << "Altitude (m)" 
                  << std::setw(12) << "Waypoint" 
                  << std::setw(12) << "Status" << "\n";
        std::cout << "---------------------------------------------------------\n";
        for (const auto& [name, state] : fleet_store_) {
            std::cout << std::left << std::setw(12) << name
                      << std::setw(12) << std::fixed << std::setprecision(1) << state.battery
                      << std::setw(15) << std::fixed << std::setprecision(1) << state.altitude
                      << std::setw(12) << state.waypoint
                      << std::setw(12) << state.status << "\n";
        }
        std::cout << "=========================================================\n\n";
    }

    void trigger_report_service(const std::shared_ptr<std_srvs::srv::Trigger::Request>,
                                std::shared_ptr<std_srvs::srv::Trigger::Response> res) {
        generate_table_report();
        res->success = true;
        res->message = "Fleet summary status report executed cleanly to dashboard console terminal output.";
    }

    std::unordered_map<std::string, DroneReportState> fleet_store_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> status_subs_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> alert_subs_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> complete_subs_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> telemetry_subs_;
    rclcpp::TimerBase::SharedPtr report_timer_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr report_service_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<FleetManager>());
    rclcpp::shutdown();
    return 0;
}
