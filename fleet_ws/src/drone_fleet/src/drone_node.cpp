#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "mission_drone.hpp"
#include <string>
#include <vector>
#include <sstream>

class DroneNode : public rclcpp::Node {
public:
    DroneNode() : Node("drone_node_instance") {
        this->declare_parameter<std::string>("drone_name", "UnknownDrone");
        this->declare_parameter<double>("initial_battery", 100.0);
        this->declare_parameter<std::string>("mission_name", "Default_Patrol");

        this->get_parameter("drone_name", drone_name_);
        this->get_parameter("initial_battery", initial_battery_);
        this->get_parameter("mission_name", mission_name_);

        std::vector<std::tuple<float, float, float>> dummy_wps = {
            {10.0, 15.0, 5.0}, {20.0, 25.0, 10.0}, {30.0, 35.0, 15.0}, {40.0, 45.0, 20.0}, {50.0, 55.0, 25.0}
        };

        drone_ptr_ = std::make_unique<MissionDrone>(drone_name_, static_cast<float>(initial_battery_), 250.0f, 5.5f, mission_name_, dummy_wps);
        drone_ptr_->take_off(10.0f + (rand() % 15)); 

        status_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + drone_name_ + "/status", 10);
        alert_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + drone_name_ + "/alert", 10);
        complete_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + drone_name_ + "/mission_complete", 10);
        telemetry_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + drone_name_ + "/telemetry", 10);

        status_timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&DroneNode::publish_status_tick, this));
        telemetry_timer_ = this->create_wall_timer(std::chrono::seconds(2), std::bind(&DroneNode::publish_telemetry_tick, this));

        publish_count_ = 0;
        is_critical_alert_fired_ = false;
    }

private:
    void publish_status_tick() {
        if (drone_ptr_->get_battery_level() <= 0.0f) {
            drone_ptr_->land();
            return;
        }

        drone_ptr_->drain_battery(0.5f);
        publish_count_++;

        if (publish_count_ % 3 == 0 && !drone_ptr_->mission_complete() && drone_ptr_->get_status() == "flying") {
            drone_ptr_->next_waypoint();
        }

        // Deliberate Custom String Serialization Format
        std::stringstream ss;
        ss << "name:" << drone_ptr_->get_name() << "|"
           << "battery:" << std::fixed << std::setprecision(1) << drone_ptr_->get_battery_level() << "|"
           << "altitude:" << std::fixed << std::setprecision(1) << drone_ptr_->get_altitude() << "|"
           << "status:" << drone_ptr_->get_status() << "|"
           << "waypoint:" << drone_ptr_->get_current_waypoint_index() << "/" << drone_ptr_->get_total_waypoints() << "|"
           << "speed:" << std::fixed << std::setprecision(1) << drone_ptr_->get_speed();

        auto msg = std_msgs::msg::String();
        msg.data = ss.str();
        status_pub_->publish(msg);

        if (drone_ptr_->is_critical() && !is_critical_alert_fired_) {
            auto alert_msg = std_msgs::msg::String();
            alert_msg.data = "CRITICAL BATTERY: Drone [" + drone_name_ + "] reached low power tier threshold!";
            alert_pub_->publish(alert_msg);
            drone_ptr_->land();
            is_critical_alert_fired_ = true;
        }

        if (drone_ptr_->mission_complete() && drone_ptr_->get_status() == "flying") {
            auto comp_msg = std_msgs::msg::String();
            comp_msg.data = "Mission Complete notice for [" + drone_name_ + "]";
            complete_pub_->publish(comp_msg);
            
            // Loop reset fallback criteria logic
            std::vector<std::tuple<float, float, float>> fresh_wps = {
                {10.0, 15.0, 5.0}, {20.0, 25.0, 10.0}, {30.0, 35.0, 15.0}, {40.0, 45.0, 20.0}, {50.0, 55.0, 25.0}
            };
            drone_ptr_ = std::make_unique<MissionDrone>(drone_name_, drone_ptr_->get_battery_level(), 250.0f, 5.5f, mission_name_, fresh_wps);
            drone_ptr_->take_off(15.0f);
        }
    }

    void publish_telemetry_tick() {
        // Manual JSON construction structure without third-party frameworks
        std::stringstream ss;
        ss << "{"
           << "\"name\":\"" << drone_ptr_->get_name() << "\","
           << "\"battery\":" << std::fixed << std::setprecision(2) << drone_ptr_->get_battery_level() << ","
           << "\"altitude\":" << std::fixed << std::setprecision(2) << drone_ptr_->get_altitude() << ","
           << "\"status\":\"" << drone_ptr_->get_status() << "\","
           << "\"waypoint_idx\":" << drone_ptr_->get_current_waypoint_index() << ","
           << "\"total_waypoints\":" << drone_ptr_->get_total_waypoints() << ","
           << "\"speed\":" << std::fixed << std::setprecision(2) << drone_ptr_->get_speed()
           << "}";

        auto msg = std_msgs::msg::String();
        msg.data = ss.str();
        telemetry_pub_->publish(msg);
    }

    std::string drone_name_;
    double initial_battery_;
    std::string mission_name_;
    std::unique_ptr<MissionDrone> drone_ptr_;

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr alert_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr complete_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr telemetry_pub_;

    rclcpp::TimerBase::SharedPtr status_timer_;
    rclcpp::TimerBase::SharedPtr telemetry_timer_;
    int publish_count_;
    bool is_critical_alert_fired_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DroneNode>());
    rclcpp::shutdown();
    return 0;
}
