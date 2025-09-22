# Linux Speed Meter

Linux Speed Meter is a cross-platform application designed to monitor and display internet upload and download speeds in real-time. It runs as a system tray application or a small window, making it compatible with popular desktop environments such as GNOME, KDE, XFCE, and Windows.

## Features

- Real-time monitoring of internet speeds
- System tray icon with tooltip displaying current speeds
- Small window option for detailed speed display
- Cross-platform compatibility (Linux and Windows)
- Minimal resource usage

## Installation

### From Pre-built Packages

Pre-built packages are available for various Linux distributions. Run the build script to create all packages:

```bash
chmod +x build_all.sh
./build_all.sh
```

Packages will be generated in the `dist/` directory with organized subdirectories for each distribution type.

#### Debian/Ubuntu
```bash
sudo dpkg -i dist/linux-speed-meter_1.0.0_amd64.deb
```

#### Fedora/RHEL
```bash
sudo rpm -i dist/linux-speed-meter-1.0.0-1.x86_64.rpm
```

#### Arch Linux
```bash
sudo pacman -U dist/linux-speed-meter-1.0.0-1-x86_64.pkg.tar.zst
```

#### AppImage (Universal)
```bash
chmod +x dist/linux-speed-meter-1.0.0-x86_64.AppImage
./dist/linux-speed-meter-1.0.0-x86_64.AppImage
```

#### Windows
```cmd
# Download the executable from releases or build from source
LinuxSpeedMeter.exe
```

### Build from Source

# Build & Run Instructions

## Prerequisites

- C++17 compatible compiler (GCC 7+ or Clang 5+)
- CMake 3.10 or later
- GTK 3.x development libraries
- libappindicator3 development libraries

### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake libgtk-3-dev libappindicator3-dev
```

### Fedora:
```bash
sudo dnf install gcc-c++ cmake gtk3-devel libappindicator-gtk3-devel
```

### Arch Linux:
```bash
sudo pacman -S base-devel cmake gtk3 libappindicator-gtk3
```

## Build with CMake (Recommended)

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd linux-speed-meter
   ```

2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Configure the project:
   ```bash
   cmake ..
   ```

4. Build the application:
   ```bash
   make
   ```

5. Run the application:
   ```bash
   ./LinuxSpeedMeter
   ```

## Windows Build Instructions

For Windows builds, see [README-Windows.md](README-Windows.md) for detailed instructions.

### Prerequisites (Windows)

- Visual Studio 2019/2022 with C++ development tools
- Qt6 (recommended) or Qt5.15+
- CMake 3.16+

### Quick Windows Build

1. Run the build script:
   ```cmd
   build_windows.bat
   ```

2. Find the executable in `build-windows\Release\LinuxSpeedMeter.exe`

## Manual Build (Alternative)

If you prefer manual compilation:

```bash
g++ src/main.cpp src/tray/tray_icon.cpp src/ui/window.cpp src/speed_monitor.cpp src/utils/helpers.cpp \
    -Iinclude -std=c++17 $(pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1) \
    -Wno-deprecated-declarations -pthread -o speed-meter
```

## Auto-Start on Login

The app automatically creates an autostart entry (`~/.config/autostart/speed-meter.desktop`) on first run. To disable, simply run and quit the app once.
- To install the binary and set up auto-start:
   ```bash
   chmod +x install.sh
   ./install.sh
   ```

## Flatpak & AppImage
- For portable distribution, see the project for Flatpak/AppImage packaging instructions.

## Compilation Instructions

1. Clone the repository:
   ```
   git clone <repository-url>
   cd linux-speed-meter
   ```

2. Create a build directory:
   ```
   mkdir build
   cd build
   ```

3. Run CMake to configure the project:
   ```
   cmake ..
   ```

4. Compile the application:
   ```
   make
   ```

## Usage

- To run the application, execute the compiled binary located in the `build` directory.
- The application will start in the system tray, displaying the current upload and download speeds.
- You can click on the tray icon to view more details or access settings.

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.