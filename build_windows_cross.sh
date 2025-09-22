#!/bin/bash

# Windows Cross-Compilation Script for Linux Speed Meter
# This creates a basic Windows executable using MinGW

echo "Setting up Windows cross-compilation..."

# Create build directory
mkdir -p build-windows
cd build-windows

echo "Building Windows executable directly with MinGW..."

# Compile directly with MinGW
x86_64-w64-mingw32-g++ \
    ../src/main_windows.cpp \
    ../src/speed_monitor.cpp \
    ../src/helpers.cpp \
    ../src/data_manager.cpp \
    -o LinuxSpeedMeter.exe \
    -std=c++14 \
    -static-libgcc \
    -static-libstdc++ \
    -mconsole \
    -liphlpapi \
    -lws2_32 \
    -I../include

# Check if build succeeded
if [ $? -eq 0 ] && [ -f "LinuxSpeedMeter.exe" ]; then
    echo "✓ Windows executable built successfully"
    
    # Create dist directory if it doesn't exist
    mkdir -p ../dist
    
    # Copy executable to dist folder
    cp LinuxSpeedMeter.exe ../dist/
    echo "✓ Windows executable copied to dist/LinuxSpeedMeter.exe"
else
    echo "✗ Failed to build Windows executable"
    exit 1
fi

echo "Build completed!"
echo "Windows executable available at: dist/LinuxSpeedMeter.exe"