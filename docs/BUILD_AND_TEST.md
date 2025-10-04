# Building and Testing Guide

## Build Instructions

### Prerequisites Check

Before building, ensure you have all required dependencies:

#### Linux (GTK Version)
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake libgtk-3-dev libappindicator3-dev git

# Fedora
sudo dnf install gcc-c++ cmake gtk3-devel libappindicator-gtk3-devel git

# Arch Linux
sudo pacman -S base-devel cmake gtk3 libappindicator-gtk3 git
```

#### Windows (Qt Version)
- Visual Studio 2019 or later
- Qt 6.x or Qt 5.15+ (with QtCharts)
- CMake 3.16+
- Git for Windows

### Building from Source

#### Linux GTK Version

```bash
# Clone the repository
git clone https://github.com/mojahid2021/linux-speed-meter.git
cd linux-speed-meter

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make -j$(nproc)

# Install (optional)
sudo make install
```

#### Windows Qt Version (Native)

```cmd
# Clone the repository
git clone https://github.com/mojahid2021/linux-speed-meter.git
cd linux-speed-meter

# Create build directory
mkdir build-windows
cd build-windows

# Configure (adjust Qt path as needed)
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64"

# Build
cmake --build . --config Release

# Executable will be in build-windows\Release\
```

#### Cross-Compilation (Windows from Linux)

```bash
# Install MinGW cross-compiler
sudo apt install mingw-w64 g++-mingw-w64-x86-64-win32

# Use the cross-compilation script
chmod +x build_windows_cross.sh
./build_windows_cross.sh

# Executable will be LinuxSpeedMeter.exe
```

### Building All Packages

```bash
# Build all distribution packages
chmod +x build_all.sh
./build_all.sh

# Packages will be in dist/ directory:
# - dist/debian/linux-speed-meter_2.0.0_amd64.deb
# - dist/rpm/linux-speed-meter-2.0.0-1.x86_64.rpm
# - dist/arch/linux-speed-meter-2.0.0-1-x86_64.pkg.tar.zst
# - dist/linux-speed-meter-2.0.0-x86_64.AppImage
```

## Testing

### Manual Testing Checklist

#### Basic Functionality
- [ ] Application starts without errors
- [ ] System tray icon appears
- [ ] Dashboard opens on icon click
- [ ] Current speeds update in real-time
- [ ] Session time increments correctly
- [ ] Total data accumulates properly

#### Settings Testing
- [ ] Refresh rate changes work (test 1s, 5s, 10s)
- [ ] Theme switching works (light/dark)
- [ ] Stay on top toggle works
- [ ] Auto-start checkbox toggles
- [ ] Notifications toggle works
- [ ] Reset statistics clears data

#### Export Testing
- [ ] Export to CSV creates valid file
- [ ] Export to JSON creates valid file
- [ ] Exported data is accurate
- [ ] File chooser works correctly
- [ ] Error messages for invalid paths

#### Chart Testing (Qt Version)
- [ ] Speed chart renders correctly
- [ ] Chart updates in real-time
- [ ] Data points are accurate
- [ ] Axes scale appropriately
- [ ] Colors match download/upload

#### GTK-Specific Testing (Linux)
- [ ] Monthly statistics display
- [ ] Data limit section works
- [ ] Interface information updates
- [ ] Export dialogs work
- [ ] System theme integration

#### Qt-Specific Testing (Windows)
- [ ] Window title shows speeds
- [ ] Taskbar preview shows speeds
- [ ] Dark theme applies correctly
- [ ] All tabs render properly
- [ ] Progress bars work

### Automated Testing

Currently, the project uses manual testing. Automated tests can be added:

```cpp
// Example unit test structure (to be implemented)
#include <gtest/gtest.h>
#include "data_exporter.h"

TEST(DataExporterTest, CSVFormatValid) {
    // Test CSV export format
    QVector<UsageRecord> records;
    // Add test data
    QString csv = DataExporter::formatCSV(records);
    // Assert CSV is valid
}
```

### Performance Testing

#### CPU Usage
```bash
# Linux
top -p $(pgrep LinuxSpeedMeter)

# Windows Task Manager or
# PowerShell: Get-Process LinuxSpeedMeter | Select CPU
```

Expected CPU usage: < 1% on modern systems

#### Memory Usage
```bash
# Linux
ps aux | grep LinuxSpeedMeter

# Windows Task Manager
```

Expected memory usage: 50-100 MB

### Network Testing

Test with different network conditions:

1. **Idle Network**: 
   - Speeds should be near 0
   - Application should remain responsive

2. **High Download**:
   - Start large download
   - Verify speed readings are accurate
   - Compare with other tools

3. **High Upload**:
   - Start large upload
   - Verify upload speed tracking
   - Check peak speed recording

4. **Simultaneous**:
   - Download and upload simultaneously
   - Both speeds should update independently

### Platform-Specific Testing

#### Ubuntu 22.04 LTS
- [ ] Build completes
- [ ] Package installs
- [ ] AppIndicator works in GNOME
- [ ] Auto-start works
- [ ] Export functions work

#### Windows 11
- [ ] Qt version builds
- [ ] System tray appears
- [ ] Taskbar integration works
- [ ] Registry auto-start works
- [ ] Export dialogs work

#### Fedora (latest)
- [ ] RPM builds
- [ ] Application runs
- [ ] System tray works
- [ ] All features functional

#### Arch Linux
- [ ] Package builds
- [ ] AUR-compatible (if published)
- [ ] All dependencies met

## Troubleshooting Build Issues

### Linux Build Errors

#### GTK Not Found
```bash
# Error: Package 'gtk+-3.0' not found
sudo apt install libgtk-3-dev

# Or check pkg-config
pkg-config --modversion gtk+-3.0
```

#### AppIndicator Not Found
```bash
# Error: Package 'appindicator3-0.1' not found
sudo apt install libappindicator3-dev
```

### Windows Build Errors

#### Qt Not Found
```cmd
# Ensure Qt is in PATH or specify CMAKE_PREFIX_PATH
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
```

#### QtCharts Missing
```
# Install QtCharts module
# In Qt Maintenance Tool, ensure Charts module is selected
```

#### MSVC Not Found
```
# Install Visual Studio Build Tools
# Or use MinGW: cmake -G "MinGW Makefiles"
```

### Cross-Compilation Issues

#### MinGW Missing
```bash
sudo apt install mingw-w64
```

#### Missing Windows Libraries
```bash
# Ensure toolchain file points to correct MinGW installation
# Edit mingw-toolchain.cmake if needed
```

## Continuous Integration

### GitHub Actions (Future)

Example workflow for automated builds:

```yaml
name: Build

on: [push, pull_request]

jobs:
  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: |
          sudo apt update
          sudo apt install libgtk-3-dev libappindicator3-dev
      - name: Build
        run: |
          mkdir build && cd build
          cmake ..
          make -j$(nproc)
  
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Setup Qt
        uses: jurplel/install-qt-action@v3
      - name: Build
        run: |
          mkdir build
          cd build
          cmake .. -G "Visual Studio 17 2022"
          cmake --build . --config Release
```

## Release Checklist

Before releasing a new version:

- [ ] Update VERSION file
- [ ] Update CHANGELOG.md with release date
- [ ] Update version in CMakeLists.txt files
- [ ] Update version in include/version.h
- [ ] Test all features on all platforms
- [ ] Build all packages successfully
- [ ] Test installation from packages
- [ ] Update documentation if needed
- [ ] Create git tag: `git tag -a v2.0.0 -m "Release 2.0.0"`
- [ ] Push tag: `git push origin v2.0.0`
- [ ] Create GitHub release with packages

## Contributing Tests

When contributing new features:

1. Add manual test cases to this document
2. Test on at least one platform thoroughly
3. Document any new dependencies
4. Update build instructions if needed
5. Include testing steps in pull request

## Version Information

This guide is for **Linux Speed Meter v2.0.0**

For issues or questions: https://github.com/mojahid2021/linux-speed-meter/issues
