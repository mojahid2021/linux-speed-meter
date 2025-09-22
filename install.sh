#!/bin/bash

# Build the application if not already built
if [ ! -f "build/LinuxSpeedMeter" ]; then
    echo "Building the application..."
    mkdir -p build
    cd build
    cmake ..
    make
    cd ..
fi

# Install binary to /usr/local/bin
install -Dm755 build/LinuxSpeedMeter /usr/local/bin/linux-speed-meter

# Create desktop entry for autostart
mkdir -p ~/.config/autostart
cat > ~/.config/autostart/linux-speed-meter.desktop << EOF
[Desktop Entry]
Type=Application
Name=Linux Speed Meter
Exec=linux-speed-meter
Icon=network-transmit-receive
Categories=Utility;Network;
StartupNotify=false
EOF

echo "Linux Speed Meter installed and set to auto-start on login."
