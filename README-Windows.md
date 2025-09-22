# Linux Speed Meter - Windows Build Instructions

This document provides instructions for building the Linux Speed Meter application for Windows using Qt.

## Prerequisites

1. **Visual Studio 2019 or 2022** with C++ development tools
2. **Qt6** (recommended) or **Qt5.15+**
   - Download from: <https://www.qt.io/download>
   - Install Qt Creator and Qt libraries for MSVC
3. **CMake 3.16+**
   - Download from: <https://cmake.org/download/>

## Build Steps

### Method 1: Using the Build Script (Recommended)

1. Open Command Prompt as Administrator
2. Navigate to the project directory:

   ```cmd
   cd path\to\linux-speed-meter
   ```

3. Run the build script:

   ```cmd
   build_windows.bat
   ```

### Method 2: Manual Build

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
