#include "vehicle.hpp"
#include "drone.hpp"
#include "mission_drone.hpp"
#include "autonomous_drone.hpp"
#include "drone_exceptions.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<std::tuple<float, float, float>> wps = { {10, 20, 15}, {30, 40, 25}, {50, 60, 35} };
    std::tuple<float, float, float> home = {0.0f, 0.0f, 0.0f};

    Drone* d = new Drone("Alpha", 85.0f, 120.0f, 12.5f);
    MissionDrone* md = new MissionDrone("Beta", 95.0f, 150.0f, 15.0f, "Survey_Grid_A", wps);
    AutonomousDrone* ad = new AutonomousDrone("Gamma", 100.0f, 200.0f, 22.0f, "Deep_Recon", wps, home);

    std::vector<Vehicle*> fleet = { d, md, ad };

    std::cout << "--- Polymorphism Demonstration ---\n";
    for (const auto& vehicle : fleet) {
        std::cout << vehicle->get_info() << "\n";
    }
    std::cout << "----------------------------------\n\n";

    // Compilation/Direct Access constraint proof:
    // Any direct access to private members such as fleet[0]->battery_level or fleet[0]->status 
    // throws an error at compile time because they are explicitly flagged private/protected.
    // Example: fleet[0]->status = "flying"; // Error: 'std::string Vehicle::status' is private within this context

    std::cout << "--- Exception Handling Checks ---\n";
    try {
        std::cout << "[Test 1] Forcing illegal charge status error...\n";
        d->charge_battery(10.0f, 5);
    } catch (const DroneException& e) {
        std::cout << "Caught expected Exception: " << e.what() << "\n";
    }

    try {
        std::cout << "[Test 2] Violating max structural altitude limit ceiling...\n";
        d->take_off(500.0f);
    } catch (const DroneException& e) {
        std::cout << "Caught expected Exception: " << e.what() << "\n";
    }

    try {
        std::cout << "[Test 3] Depleting battery systematically down to point-zero error...\n";
        d->drain_battery(100.0f);
        d->drain_battery(5.0f); // Should trigger exception
    } catch (const DroneException& e) {
        std::cout << "Caught expected Exception: " << e.what() << "\n";
    }

    std::cout << "\n--- Full Autonomous Mission Simulation ---\n";
    ad->take_off(50.0f);
    std::cout << "Mission Operational Status. Proceeding along path waypoints...\n";
    
    while (!ad->mission_complete()) {
        ad->next_waypoint();
    }

    std::cout << "Simulating immediate high-severity safety critical hazard detection...\n";
    ad->detect_obstacle({32.0f, 42.0f, 25.0f}, "high");

    std::cout << "\n" << ad->mission_summary() << "\n";
    std::cout << "--- Drone Flight Log ---\n" << ad->get_flight_log();

    // Cleanup
    for (auto v : fleet) {
        delete v;
    }

    return 0;
}
