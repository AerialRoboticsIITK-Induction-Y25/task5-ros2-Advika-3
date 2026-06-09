#ifndef DRONE_HPP
#define DRONE_HPP

#include "vehicle.hpp"

class Drone : public Vehicle {
public:
    Drone(const std::string& name, float battery_level, float max_altitude, float speed);
    virtual ~Drone() = default;

    std::string get_info() const override;
    
    virtual void take_off(float target_altitude);
    virtual void land();
    void emergency_stop();

    float get_altitude() const;
    float get_max_altitude() const;
    float get_speed() const;

protected:
    float altitude;
    float max_altitude;

private:
    float speed;
};

#endif // DRONE_HPP
