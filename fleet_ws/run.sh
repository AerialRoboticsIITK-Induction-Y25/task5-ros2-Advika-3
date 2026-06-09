#!/bin/bash
set -e

# Resolve the absolute workspace directory root paths path 
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
WORKSPACE_DIR="$(dirname "$SCRIPT_DIR")"

echo "=========================================================="
echo " Building Optimized Docker Image Asset Layer [ros:jazzy]  "
echo "=========================================================="
cd "$WORKSPACE_DIR"
docker build -t virtual_drone_fleet:latest -f fleet_ws/Dockerfile .

echo ""
echo "=========================================================="
echo " Running Virtual Drone Fleet Manager Node Stack Container "
echo "=========================================================="
# Runs with host networking, sets isolated domain to 42, auto-removes on completion
docker run -it --rm \
    --net=host \
    -e ROS_DOMAIN_ID=42 \
    --name ariitk_fleet_container \
    virtual_drone_fleet:latest
