#!/bin/bash

set -e

BUILD_DIR="build"

killJobs() {
    for job in $(jobs -p); do
        kill -s SIGTERM $job > /dev/null 2>&1 || (sleep 10 && kill -9 $job > /dev/null 2>&1 &)
    done
}

trap killJobs SIGINT SIGTERM EXIT

# Executable paths
NODES=(
    "$BUILD_DIR/AutoLights_AutoLights"
    "$BUILD_DIR/AutoLights_Timer"
    "$BUILD_DIR/AutoLights_FrontCamera"
    "$BUILD_DIR/AutoLights_HighBeams"
    "$BUILD_DIR/AutoLights_LightSensor"
)

CTL="$BUILD_DIR/AutoLights_Controller"



for exe in "${NODES[@]}"; do
    if [[ -x "$exe" ]]; then
        "$exe" &
        echo "Started $exe"
    else
        echo "Executable not found or not built: $exe"
    fi
done

# Run the controller in the foreground if it exists
if [[ -x "$CTL" ]]; then
    echo "Starting $CTL"
    "$CTL"
else
    echo "Executable not found or not built: $CTL"
    wait
fi

echo $(jobs -p)


wait