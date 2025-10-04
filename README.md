# Linux Speed Meter

> **🎉 NEW: Critical Stability Update** - Speed test crashes fixed! See [UPGRADE_GUIDE.md](docs/UPGRADE_GUIDE.md) for details.

Linux Speed Meter is a cross-platform application designed to monitor and display internet upload and download speeds in real-time. It runs as a system tray application or a small window, making it compatible with popular desktop environments such as GNOME, KDE, XFCE, and Windows.

## Recent Improvements (Latest Release)

✅ **Speed Test Stability** - Fixed all crash issues in speed test tab  
✅ **Thread Safety** - Proper synchronization for UI updates  
✅ **Memory Management** - RAII wrappers prevent memory leaks  
✅ **Error Handling** - Graceful handling of network errors with retry logic  
✅ **Build System** - Enhanced cross-platform support and optimization  

For full details, see [docs/SPEED_TEST_FIXES.md](docs/SPEED_TEST_FIXES.md)

## Version

**Current Version: 2.0.0**

See [CHANGELOG.md](CHANGELOG.md) for detailed version history and release notes.

## Features

- **Real-time monitoring** of internet speeds with live updates
- **Active Speed Testing** - NEW! Test your actual internet connection speed
  - Download speed test with parallel connections
  - Upload speed test with accurate measurements
  - Latency (ping) testing with min/avg/max metrics
  - Jitter measurement for connection quality
  - Multiple test servers (Cloudflare, Fast.com, LibreSpeed)
  - Beautiful GUI with progress indicators
- **System tray integration** with tooltip displaying current speeds
- **Comprehensive Dashboard** with detailed usage statistics:
  - Current download/upload speeds
  - Session time and total data transferred
  - Average speeds calculation
  - Network interface information
  - Peak speed tracking
  - **Monthly Statistics**: Total data usage, peak speeds, active days
  - **Data Limits**: Configurable monthly limits with usage warnings
  - **Persistent Data**: Statistics saved across sessions
- **Cross-platform compatibility** (Linux GTK and Windows Qt GUI)
- **Advanced Features** (New in v2.0):
  - **Dark Mode Theme**: Modern dark theme for better viewing experience
  - **Data Export**: Export usage statistics to CSV or JSON formats
  - **Customizable Refresh Rate**: Adjust update frequency (1-60 seconds)
  - **Notifications**: Alerts for connection status and data limits
  - **Windows Taskbar Integration**: Real-time speed display in window title
  - **Historical Data Tracking**: Keep last 1000 data points for analysis
- **Minimal resource usage** with efficient monitoring
- **Auto-start functionality** on system boot
- **Cross-compilation support** - Build Windows .exe from Linux

## Documentation

- **[docs/UPGRADE_GUIDE.md](docs/UPGRADE_GUIDE.md)** - 🆕 Upgrade guide for latest stability fixes
- **[docs/SPEED_TEST_FIXES.md](docs/SPEED_TEST_FIXES.md)** - 🆕 Technical details of crash fixes
- **[docs/SPEED_TEST_GUIDE.md](docs/SPEED_TEST_GUIDE.md)** - Complete speed test feature guide
- **[docs/USER_GUIDE.md](docs/USER_GUIDE.md)** - Complete user guide with tutorials and troubleshooting
- **[docs/FEATURES.md](docs/FEATURES.md)** - Detailed feature documentation and specifications
- **[CHANGELOG.md](CHANGELOG.md)** - Version history and release notes
- **[README-Windows.md](README-Windows.md)** - Windows-specific build instructions
- **[docs/BUILD_AND_TEST.md](docs/BUILD_AND_TEST.md)** - Build and test guide for developers
- **[docs/CONTRIBUTING.md](docs/CONTRIBUTING.md)** - Contribution guidelines

## Quick Links

- [Installation](#installation)
- [Building from Source](#build-with-cmake-recommended)
- [Usage Guide](docs/USER_GUIDE.md)
- [Features Overview](docs/FEATURES.md)
- [Contributing](docs/CONTRIBUTING.md)
- [Changelog](#changelog)

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
sudo dpkg -i dist/linux-speed-meter_2.0.0_amd64.deb
```

#### Fedora/RHEL
```bash
sudo rpm -i dist/linux-speed-meter-2.0.0-1.x86_64.rpm
```

#### Arch Linux
```bash
sudo pacman -U dist/linux-speed-meter-2.0.0-1-x86_64.pkg.tar.zst
```

#### AppImage (Universal)
```bash
chmod +x dist/linux-speed-meter-2.0.0-x86_64.AppImage
./dist/linux-speed-meter-2.0.0-x86_64.AppImage
```

#### Windows (Cross-compiled from Linux)
```bash
# Build Windows .exe from Linux Ubuntu
chmod +x build_windows_cross.sh
./build_windows_cross.sh
# Find LinuxSpeedMeter.exe in the project root
```

### Build from Source

## Prerequisites

- **Linux Build:**
  - C++14 compatible compiler (GCC 7+ or Clang 5+)
  - CMake 3.10 or later
  - GTK 3.x development libraries
  - libappindicator3 development libraries

- **Windows Cross-compilation:**
  - MinGW-w64 (automatically installed by setup script)
  - Same Linux prerequisites

### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake libgtk-3-dev libappindicator3-dev libcurl4-openssl-dev
# For Windows cross-compilation:
sudo apt install mingw-w64 g++-mingw-w64-x86-64-win32
```

### Fedora:
```bash
sudo dnf install gcc-c++ cmake gtk3-devel libappindicator-gtk3-devel libcurl-devel
```

### Arch Linux:
```bash
sudo pacman -S base-devel cmake gtk3 libappindicator-gtk3 curl
```

## Build with CMake (Recommended)

### Linux Build

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

### Windows Cross-compilation from Linux

1. Set up cross-compilation environment:
   ```bash
   chmod +x setup_dev.sh
   ./setup_dev.sh
   ```

2. Build Windows executable:
   ```bash
   chmod +x build_windows_cross.sh
   ./build_windows_cross.sh
   ```

3. The `LinuxSpeedMeter.exe` will be created in the project root

### Manual Build (Alternative)

#### Linux
```bash
g++ src/main.cpp src/tray_icon.cpp src/window.cpp src/speed_monitor.cpp src/helpers.cpp src/data_manager.cpp \
    -Iinclude -std=c++14 $(pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1) \
    -Wno-deprecated-declarations -pthread -o speed-meter
```

#### Windows (Direct cross-compilation)
```bash
x86_64-w64-mingw32-g++ -std=c++14 -Iinclude -Isrc -D_WIN32 -DWIN32 \
    src/main_windows.cpp src/data_manager.cpp src/helpers.cpp \
    -o LinuxSpeedMeter.exe -liphlpapi -lws2_32 -static-libgcc -static-libstdc++
```

### Cleanup Build Artifacts

To clean up build artifacts, temporary files, and packaging outputs:

```bash
./cleanup.sh
```

This script safely removes:

- Build directories (`build/`, `build-windows/`, etc.)
- CMake cache files and build artifacts
- Packaging outputs in `dist/`
- Temporary files and backups
- Empty directories

**Note:** The cleanup script automatically handles permission issues and will use `sudo` when necessary to remove files owned by root.

## Windows Build Instructions

For Windows builds, you have two options:

### Option 1: Cross-compile from Linux (Recommended)
```bash
# From Linux Ubuntu system
./build_windows_cross.sh
# Creates LinuxSpeedMeter.exe (Windows console application)
```

### Option 2: Native Windows Build
For native Windows builds with Qt GUI, see [README-Windows.md](README-Windows.md) for detailed instructions using Visual Studio and Qt.

## Usage

### System Tray Operation
- The application starts in the system tray, displaying current upload and download speeds
- Right-click the tray icon to access the menu

### Dashboard Access
- **GTK Version (Linux)**: Select "Show Dashboard" from the tray menu
- **Windows Console Version**: Run `LinuxSpeedMeter.exe` directly for console monitoring

### Dashboard Features (Linux GTK)

#### Current Speed Section
- Real-time download and upload speeds
- Color-coded speed indicators

#### Session Statistics
- Session duration tracking
- Total data transferred (download/upload)
- Average speeds calculation
- Peak speed monitoring

#### Monthly Statistics (New!)
- Total monthly data usage
- Peak download/upload speeds
- Average daily usage
- Number of active days
- Data usage percentage with warnings

#### Data Limit Management
- Configurable monthly data limits
- Usage percentage indicators
- Warning alerts when approaching limits

#### Network Interface Information
- Active interface detection
- IP address display
- Connection status

### Windows Console Version Features

The Windows executable (`LinuxSpeedMeter.exe`) provides:

- **Real-time speed monitoring** in console
- **Persistent data storage** to `%APPDATA%\linux-speed-meter\usage_data.txt`
- **Automatic data saving** every minute
- **Cross-platform data format** compatible with Linux version

#### Console Output Format:
```
Download | Upload | Total Down | Total Up
---------|--------|------------|----------
1.2 MB/s | 0.3 MB/s| 45.2 MB   | 12.1 MB
```

## Data Persistence

### Linux
- Data stored in: `~/.config/linux-speed-meter/usage_data.txt`
- Statistics persist across application restarts
- Monthly data tracking with historical records

### Windows
- Data stored in: `%APPDATA%\linux-speed-meter\usage_data.txt`
- Compatible data format with Linux version
- Cross-platform data sharing support

### Data Format
```
0
2025-09-22,1000000,500000,1000,500,1,60
```
Format: date,total_download_bytes,total_upload_bytes,peak_download_speed,peak_upload_speed,session_count,total_session_time_seconds

## Auto-Startup Configuration

Linux Speed Meter supports automatic startup on system boot across various Linux distributions and desktop environments. The application uses multiple fallback methods to ensure compatibility:

### Supported Methods

1. **XDG Autostart** (Most desktop environments)
   - Creates: `~/.config/autostart/linux-speed-meter.desktop`
   - Compatible with: GNOME, KDE, XFCE, LXDE, MATE, Cinnamon, Unity

2. **Systemd User Service** (Modern Linux systems)
   - Creates: `~/.config/systemd/user/linux-speed-meter.service`
   - Enables automatic restart on failure
   - Compatible with: systemd-based distributions (Ubuntu 16.04+, Fedora, Arch, etc.)

3. **Cron Job** (Fallback for older systems)
   - Creates: `~/.crontab_speed_meter`
   - Uses `@reboot` directive for startup
   - Compatible with: All systems with cron installed

4. **Desktop-Specific Configuration**
   - KDE: `~/.kde/Autostart/linux-speed-meter.desktop`
   - GNOME/XFCE/LXDE/MATE/Cinnamon: Uses XDG autostart

### Enabling Auto-Startup

#### GTK Version (Linux)

Auto-startup is automatically configured when the application starts. The system will:

1. Detect your desktop environment
2. Create appropriate autostart entries
3. Enable systemd services where available

### Manual Configuration

If automatic setup fails, you can manually configure auto-startup:

#### XDG Autostart (Manual)

```bash
mkdir -p ~/.config/autostart
cat > ~/.config/autostart/linux-speed-meter.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=Linux Speed Meter
Comment=Network speed monitoring application
Exec=linux-speed-meter
Icon=network-transmit-receive
Categories=Utility;Network;Monitor;
StartupNotify=false
Terminal=false
X-GNOME-Autostart-enabled=true
X-KDE-autostart-after=panel
X-MATE-Autostart-Delay=0
EOF
```

#### Systemd Service (Manual)

```bash
mkdir -p ~/.config/systemd/user
cat > ~/.config/systemd/user/linux-speed-meter.service << 'EOF'
[Unit]
Description=Linux Speed Meter
After=network.target

[Service]
ExecStart=/usr/local/bin/linux-speed-meter
Restart=always
RestartSec=5

[Install]
WantedBy=default.target
EOF
systemctl --user daemon-reload
systemctl --user enable linux-speed-meter.service
```

### Disabling Auto-Startup

The application automatically manages auto-startup. To disable:

1. The auto-startup entries will be removed when the application is uninstalled
2. Or manually remove the files created above

### Troubleshooting

- **Application doesn't start automatically**: Check if the executable path in the desktop files matches your installation
- **Systemd service fails**: Run `systemctl --user status linux-speed-meter.service` for details
- **Permission issues**: Ensure the autostart files have execute permissions
- **Multiple entries**: The application creates multiple entries for compatibility - this is normal

## Development Setup

### Automated Setup

Run the development setup script to configure your environment:

```bash
chmod +x setup_dev.sh
./setup_dev.sh
```

This script will:
- Detect your Linux distribution
- Install all required dependencies
- Set up the development environment
- Configure cross-compilation tools for Windows builds

### Manual Development Setup

1. Install dependencies (see Prerequisites section)
2. Clone the repository
3. Build using CMake (see Build from Source)

## Project Structure

```
linux-speed-meter/
├── src/                    # Source code
│   ├── main.cpp           # Linux GTK application entry point
│   ├── main_qt.cpp        # Qt application entry point (Windows/Linux)
│   ├── main_windows.cpp   # Windows console application entry point
│   ├── speed_monitor.cpp  # Network speed monitoring
│   ├── speed_monitor_linux.cpp # Linux-specific monitoring
│   ├── speed_monitor_win.cpp   # Windows-specific monitoring
│   ├── tray_icon.cpp      # GTK system tray integration
│   ├── window.cpp         # GTK dashboard window
│   ├── mainwindow.cpp     # Qt main window
│   ├── systemtray.cpp     # Qt system tray
│   ├── data_exporter.cpp  # CSV/JSON export functionality
│   ├── data_manager.cpp   # Data persistence and statistics
│   └── helpers.cpp        # Utility functions
├── include/               # Header files
│   ├── mainwindow.h
│   ├── systemtray.h
│   ├── data_exporter.h
│   ├── version.h          # Version definitions
│   └── ...
├── packaging/             # Package build scripts for different distributions
├── docs/                  # Documentation
│   ├── USER_GUIDE.md      # User manual
│   ├── FEATURES.md        # Feature documentation
│   ├── BUILD_AND_TEST.md  # Build and test guide
│   ├── CONTRIBUTING.md    # Contribution guidelines
│   └── SCREENSHOTS.md     # Visual documentation
├── build_all.sh           # Master build script for all packages
├── build_windows_cross.sh # Windows cross-compilation script
├── setup_dev.sh           # Development environment setup
├── cleanup.sh             # Cleanup script
├── CMakeLists.txt         # CMake build configuration (Linux GTK)
├── CMakeLists-Windows.txt # CMake build configuration (Windows Qt)
├── mingw-toolchain.cmake  # MinGW cross-compilation toolchain
├── resources.qrc          # Qt resources
├── VERSION                # Version number file
├── CHANGELOG.md           # Version history
└── README.md              # This file
```

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.

See [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md) for detailed contribution guidelines.

### Development Guidelines

1. **Code Style**: Follow C++14 standards with clear, documented code
2. **Cross-platform**: Ensure code works on both Linux and Windows
3. **Testing**: Test builds on target platforms before submitting
4. **Documentation**: Update documentation for any new features

### Quick Start for Contributors

```bash
# Fork and clone
git clone https://github.com/YOUR-USERNAME/linux-speed-meter.git
cd linux-speed-meter

# Set up development environment
./setup_dev.sh

# Create feature branch
git checkout -b feature/my-feature

# Make changes, test, and submit PR
```

See [docs/BUILD_AND_TEST.md](docs/BUILD_AND_TEST.md) for detailed build instructions.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.

## Changelog

### Version 2.0.0 (Current)
- ✅ **Windows GUI Application**: Full Qt-based GUI with system tray
- ✅ **Dark Mode Theme**: Modern theme support for better user experience
- ✅ **Data Export**: Export statistics to CSV and JSON formats
- ✅ **Customizable Settings**: Adjustable refresh rates and display options
- ✅ **Notification System**: Alerts for data limits and connection issues
- ✅ **Windows Taskbar Integration**: Real-time speed display in window title
- ✅ **Enhanced Linux GUI**: Export buttons and improved user interface
- ✅ **Historical Data Tracking**: Track and export up to 1000 data points
- ✅ **Version Management**: Centralized VERSION file and CHANGELOG.md

### Version 1.0.0
- ✅ **Cross-compilation support**: Build Windows .exe from Linux Ubuntu
- ✅ **Enhanced dashboard**: Monthly statistics and data limits
- ✅ **Data persistence**: Statistics saved across sessions
- ✅ **Improved GTK dashboard**: Better UI with monthly stats
- ✅ **Windows console version**: Native Windows speed monitoring
- ✅ **Comprehensive build system**: Support for multiple package formats
- ✅ **Development automation**: Automated setup and build scripts

### Previous Features
- Real-time network speed monitoring
- System tray integration
- GTK dashboard with basic statistics
- Auto-startup configuration
- Multi-distribution packaging support
```bash
chmod +x dist/linux-speed-meter-1.0.0-x86_64.AppImage
./dist/linux-speed-meter-1.0.0-x86_64.AppImage
```

#### Windows
```cmd
# Download the executable from releases or build from source
LinuxSpeedMeter.exe
```
### Unnecessary commands
--- IGNORE ---
--- IGNORE ---
```
mkdir -p build && cd build && cmake .. && make
```
--- IGNORE ---
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

### Cleanup Build Artifacts

To clean up build artifacts, temporary files, and packaging outputs:

```bash
./cleanup.sh
```

This script safely removes:

- Build directories (`build/`, `build-qt/`, etc.)
- CMake cache files and build artifacts
- Packaging outputs in `dist/`
- Temporary files and backups
- Empty directories

**Note:** The cleanup script automatically handles permission issues and will use `sudo` when necessary to remove files owned by root.

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

### System Tray Operation
- The application starts in the system tray, displaying current upload and download speeds
- Right-click the tray icon to access the menu

### Dashboard Access
- **GTK Version (Linux)**: Select "Show Dashboard" from the tray menu
- **Qt Version (Windows/Cross-platform)**: Select "Show Dashboard" from the tray menu

### Dashboard Features

#### Overview Tab (Qt Version)
- **Current Speeds**: Real-time download and upload speeds with color coding
- **Session Information**: Session duration, total data transferred, average speeds
- **Network Interface**: Interface name, IP address, connection status with visual indicators

#### Statistics Tab (Qt Version)
- **Detailed Statistics**: Peak speeds, total downloaded/uploaded data
- **Usage Summary**: Progress bars showing data usage patterns
- **Performance Metrics**: Historical peak tracking

#### Charts Tab (Qt Version)
- **Speed Over Time**: Interactive line chart showing speed history (last 60 seconds)
- **Data Usage Visualization**: Total data transfer breakdown

#### Settings Tab (Qt Version)
- **Auto-start**: Enable/disable automatic startup on system boot
- **Stay on Top**: Keep dashboard window always visible
- **Reset Statistics**: Clear all session data and charts

### GTK Dashboard Features (Linux)
- **Current Speeds**: Real-time speed monitoring
- **Session Statistics**: Time, total data, average speeds
- **Network Info**: Interface details and connection status
- **Reset Function**: Clear statistics with reset button

## Auto-Startup Configuration

Linux Speed Meter supports automatic startup on system boot across various Linux distributions and desktop environments. The application uses multiple fallback methods to ensure compatibility:

### Supported Methods

1. **XDG Autostart** (Most desktop environments)
   - Creates: `~/.config/autostart/linux-speed-meter.desktop`
   - Compatible with: GNOME, KDE, XFCE, LXDE, MATE, Cinnamon, Unity

2. **Systemd User Service** (Modern Linux systems)
   - Creates: `~/.config/systemd/user/linux-speed-meter.service`
   - Enables automatic restart on failure
   - Compatible with: systemd-based distributions (Ubuntu 16.04+, Fedora, Arch, etc.)

3. **Cron Job** (Fallback for older systems)
   - Creates: `~/.crontab_speed_meter`
   - Uses `@reboot` directive for startup
   - Compatible with: All systems with cron installed

4. **Desktop-Specific Configuration**
   - KDE: `~/.kde/Autostart/linux-speed-meter.desktop`
   - GNOME/XFCE/LXDE/MATE/Cinnamon: Uses XDG autostart

### Enabling Auto-Startup

#### GTK Version (Linux)

Auto-startup is automatically configured when the application starts. The system will:

1. Detect your desktop environment
2. Create appropriate autostart entries
3. Enable systemd services where available

#### Qt Version (Cross-platform)

1. Open the dashboard
2. Go to the Settings tab
3. Check "Enable Auto-start on Boot"
4. The application will configure auto-startup for your platform

### Manual Configuration

If automatic setup fails, you can manually configure auto-startup:

#### XDG Autostart (Manual)

```bash
mkdir -p ~/.config/autostart
cat > ~/.config/autostart/linux-speed-meter.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=Linux Speed Meter
Comment=Network speed monitoring application
Exec=linux-speed-meter
Icon=network-transmit-receive
Categories=Utility;Network;Monitor;
StartupNotify=false
Terminal=false
X-GNOME-Autostart-enabled=true
X-KDE-autostart-after=panel
X-MATE-Autostart-Delay=0
EOF
```

#### Systemd Service (Manual)

```bash
mkdir -p ~/.config/systemd/user
cat > ~/.config/systemd/user/linux-speed-meter.service << 'EOF'
[Unit]
Description=Linux Speed Meter
After=network.target

[Service]
ExecStart=/usr/local/bin/linux-speed-meter
Restart=always
RestartSec=5

[Install]
WantedBy=default.target
EOF
systemctl --user daemon-reload
systemctl --user enable linux-speed-meter.service
```

### Disabling Auto-Startup

#### GTK Version

The application automatically manages auto-startup. To disable:

1. The auto-startup entries will be removed when the application is uninstalled
2. Or manually remove the files created above

#### Qt Version

1. Open the dashboard
2. Go to the Settings tab
3. Uncheck "Enable Auto-start on Boot"

### Troubleshooting

- **Application doesn't start automatically**: Check if the executable path in the desktop files matches your installation
- **Systemd service fails**: Run `systemctl --user status linux-speed-meter.service` for details
- **Permission issues**: Ensure the autostart files have execute permissions
- **Multiple entries**: The application creates multiple entries for compatibility - this is normal

### Keyboard Shortcuts

- **Ctrl+Q**: Quit application (when dashboard is focused)
- **F5**: Refresh data (Qt version)

### Data Persistence

- Statistics reset when application restarts
- Settings are preserved between sessions

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.
