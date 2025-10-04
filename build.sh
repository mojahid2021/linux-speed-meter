#!/bin/bash
# Quick build script for development

set -e

echo "Building Linux Speed Meter..."

# Clean and rebuild
if [ "$1" = "clean" ]; then
    rm -rf build
fi

mkdir -p build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
make -j$(nproc)

echo "Build completed! Run ./build/LinuxSpeedMeter to test."