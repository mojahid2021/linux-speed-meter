#!/bin/bash
# Quick run script for development

if [ -f "build/LinuxSpeedMeter" ]; then
    echo "Running Linux Speed Meter..."
    ./build/LinuxSpeedMeter
else
    echo "Binary not found. Run ./build.sh first."
    exit 1
fi
