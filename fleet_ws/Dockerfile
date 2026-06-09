# Stage 1: Build Workspace
FROM ros:jazzy-ros-base AS builder

SHELL ["/bin/bash", "-c"]

WORKDIR /workspace_build

# Copy Part 1 (OOP Classes dependency code base relative path alignment)
COPY part1/ /part1/

# Copy ROS 2 package sources layout
COPY fleet_ws/src/ /workspace_build/src/

# Install missing system keys if required and source dependencies build pipeline
RUN apt-get update && rosdep update && \
    rosdep install --from-paths src --ignore-src -y --rosdistro jazzy && \
    source /opt/ros/jazzy/setup.bash && \
    colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release

# Stage 2: Minimal Runtime Target
FROM ros:jazzy-ros-core

SHELL ["/bin/bash", "-c"]

WORKDIR /fleet_runtime_ws

# Copy built setup runtime artifacts from builder image layer profile 
COPY --from=builder /workspace_build/install/ /fleet_runtime_ws/install/

# Inject setup automatic entry points within terminal interactive sessions
RUN echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc && \
    echo "source /fleet_runtime_ws/install/setup.bash" >> ~/.bashrc

# Entrypoint setup definition script block
ENTRYPOINT ["/bin/bash", "-c", "source /opt/ros/jazzy/setup.bash && source /fleet_runtime_ws/install/setup.bash && ros2 launch drone_fleet fleet.launch.py"]
