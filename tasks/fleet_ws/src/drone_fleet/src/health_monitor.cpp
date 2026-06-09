#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <sstream>
#include <iomanip>
#include <iostream>

struct HealthSample {
    std::chrono::steady_clock::time_point time;
    float battery;
};

class HealthMonitor : public rclcpp::Node {
public:
    HealthMonitor() : Node("health_monitor") {
        std::vector<std::string> fleet = {"Alpha", "Beta", "Gamma"};

        for (const auto& name : fleet) {
            telemetry_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/telemetry", 10,
                [this, name](const std_msgs::msg::String::SharedPtr msg) { this->process_telemetry(name, msg->data); }));
        }

        warning_pub_ = this->create_publisher<std_msgs::msg::String>("/fleet/health_warning", 10);
        summary_pub_ = this->create_publisher<std_msgs::msg::String>("/fleet/health_summary", 10);

        diagnostics_timer_ = this->create_wall_timer(std::chrono::seconds(10), std::bind(&HealthMonitor::run_diagnostics_tick, this));
    }

private:
    void process_telemetry(const std::string& name, const std::string& json) {
        float current_battery = std::stof(extract_json_field(json, "battery"));
        auto now = std::chrono::steady_clock::now();

        auto& buffer = window_buffers_[name];
        buffer.push_back(HealthSample{now, current_battery});

        if (buffer.size() > 10) {
            buffer.pop_front();
        }

        if (buffer.size() >= 2) {
            float rate = calculate_drain_rate(buffer);
            if (rate > 1.5f) {
                auto alert_msg = std_msgs::msg::String();
                alert_msg.data = "WARNING: Critical power discharge rate anomaly noticed on [" + name + "]: " + 
                                 std::to_string(rate) + "% per sec.";
                warning_pub_->publish(alert_msg);
            }
        }
    }

    float calculate_drain_rate(const std::deque<HealthSample>& buffer) {
        if (buffer.size() < 2) return 0.0f;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(buffer.back().time - buffer.front().time).count() / 1000.0f;
        if (duration <= 0.0f) return 0.0f;
        float depletion = buffer.front().battery - buffer.back().battery;
        return (depletion > 0.0f) ? (depletion / duration) : 0.0f;
    }

    std::string extract_json_field(const std::string& json, const std::string& key) {
        size_t key_pos = json.find("\"" + key + "\"");
        if (key_pos == std::string::npos) return "0";
        size_t start_pos = json.find(":", key_pos);
        start_pos++;
        while (json[start_pos] == ' ' || json[start_pos] == '"') start_pos++;
        size_t end_pos = start_pos;
        while (json[end_pos] != ',' && json[end_pos] != '}' && json[end_pos] != '"') end_pos++;
        return json.substr(start_pos, end_pos - start_pos);
    }

    void run_diagnostics_tick() {
        std::cout << "\n=========================================================\n";
        std::cout << "               FLEET HEALTH & DIAGNOSTICS                \n";
        std::cout << "=========================================================\n";
        std::cout << std::left << std::setw(10) << "Drone" 
                  << std::setw(15) << "Drain/Sec" 
                  << std::setw(18) << "Time To Crit(20%)" 
                  << std::setw(15) << "Time To Empty" << "\n";
        std::cout << "---------------------------------------------------------\n";

        std::stringstream json_summary;
        json_summary << "{";

        std::vector<std::string> fleet = {"Alpha", "Beta", "Gamma"};
        for (size_t i = 0; i < fleet.size(); ++i) {
            std::string name = fleet[i];
            auto& buffer = window_buffers_[name];
            float rate = calculate_drain_rate(buffer);
            float current_bat = buffer.empty() ? 0.0f : buffer.back().battery;

            float time_to_crit = (current_bat > 20.0f && rate > 0.0f) ? ((current_bat - 20.0f) / rate) : 0.0f;
            float time_to_empty = (current_bat > 0.0f && rate > 0.0f) ? (current_bat / rate) : 0.0f;

            std::cout << std::left << std::setw(10) << name
                      << std::setw(15) << std::fixed << std::setprecision(2) << rate
                      << std::setw(18) << (time_to_crit > 0.0f ? std::to_string(static_cast<int>(time_to_crit)) + "s" : "N/A")
                      << std::setw(15) << (time_to_empty > 0.0f ? std::to_string(static_cast<int>(time_to_empty)) + "s" : "N/A") << "\n";

            json_summary << "\"" << name << "\":{\"drain_rate\":" << rate << ",\"ttc\":" << time_to_crit << ",\"tte\":" << time_to_empty << "}";
            if (i < fleet.size() - 1) json_summary << ",";
        }
        std::cout << "=========================================================\n\n";
        json_summary << "}";

        auto summary_msg = std_msgs::msg::String();
        summary_msg.data = json_summary.str();
        summary_pub_->publish(summary_msg);
    }

    std::unordered_map<std::string, std::deque<HealthSample>> window_buffers_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> telemetry_subs_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr warning_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr summary_pub_;
    rclcpp::TimerBase::SharedPtr diagnostics_timer_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<HealthMonitor>());
    rclcpp::shutdown();
    return 0;
}
