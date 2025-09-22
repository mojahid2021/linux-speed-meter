# Linux Speed Meter - Windows Build Instructions

This document provides instructions for building the Linux Speed Meter application for Windows using multiple methods.

## Build Methods

### Method 1: Cross-compile from Linux (Recommended)

The easiest way to build Windows executables is to cross-compile from a Linux Ubuntu system.

#### Prerequisites (Linux)
- Ubuntu 18.04 or later
- Internet connection for downloading tools

#### Automated Setup
```bash
# Clone the repository
git clone <repository-url>
cd linux-speed-meter

# Run development setup (includes MinGW installation)
chmod +x setup_dev.sh
./setup_dev.sh

# Build Windows executable
chmod +x build_windows_cross.sh
./build_windows_cross.sh
```

#### Manual Setup (if automated setup fails)
```bash
# Install MinGW cross-compilation tools
sudo apt update
sudo apt install mingw-w64 g++-mingw-w64-x86-64-win32 binutils-mingw-w64 cmake

# Build Windows executable
chmod +x build_windows_cross.sh
./build_windows_cross.sh
```

#### What you get:
- `LinuxSpeedMeter.exe` - Windows console application (2.8 MB)
- Native Windows speed monitoring
- Cross-platform data persistence
- No external dependencies required

### Method 2: Native Windows Build with Qt GUI

For a full GUI application with Qt interface, build natively on Windows.

1. Create a build directory:

   ```cmd
   mkdir build-windows
   cd build-windows
   ```

2. Configure with CMake:

   ```cmd
   cmake -G "Visual Studio 16 2019" -A x64 .. -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64"
   ```

   *Note: Adjust the Qt path according to your installation*

3. Build the project:

   ```cmd
   cmake --build . --config Release
   ```

## Output

The executable will be created at:

```text
build-windows\Release\LinuxSpeedMeter.exe
```

## Deployment

The build script automatically deploys Qt DLLs to the output directory. To distribute the application:

1. Copy the entire `Release` folder
2. The application is now portable and can run on any Windows 10/11 system

## Features

- Real-time network speed monitoring
- System tray integration
- Minimal resource usage
- Cross-platform compatible (same codebase works on Linux)

## Troubleshooting

### Qt Not Found

- Ensure Qt is installed and the `CMAKE_PREFIX_PATH` points to the correct Qt installation
- Check that the Qt bin directory is in your PATH

### Build Errors

- Ensure Visual Studio Build Tools are installed
- Try building in a clean build directory

### Runtime Issues

- Make sure all Qt DLLs are in the same directory as the executable
- Check Windows Firewall settings if network monitoring fails
