# Changelog

All notable changes to Linux Speed Meter will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.1] - 2024-10-05

### Fixed - Critical Stability Update
- **Speed Test Crash Fix**: Fixed critical crashes in speed test tab
  - Proper CURL global initialization for thread safety
  - Added mutex protection for all UI updates from worker threads
  - Comprehensive null pointer checks before GTK widget access
  - Retry logic for transient network failures (max 3 attempts)
  - Fixed race conditions in test cancellation logic

### Added
- **RAII Pattern**: CurlHandle wrapper class for automatic resource cleanup
  - Prevents memory leaks in all network operations
  - Automatic cleanup even with exceptions
  - Applied to download, upload, and ping tests
- **Enhanced Error Handling**:
  - Pre-connectivity check before full speed test
  - URL validation before testing
  - More informative error messages with server names
  - Better progress indicators (0-100%)
- **Documentation**:
  - docs/SPEED_TEST_FIXES.md - Technical details of all fixes
  - docs/UPGRADE_GUIDE.md - User-friendly upgrade guide

### Enhanced
- **Build System**:
  - CMake platform detection (Windows/macOS/Linux)
  - build.sh now supports clean, debug, and release modes
  - Release builds use -O3 -march=native optimization
  - build_all.sh tracks success/failure counts
- **Network Configuration**:
  - SSL verification disabled for speed test endpoints (standard practice)
  - Improved timeout handling
  - Better connection error recovery

### Performance
- Optimized Release builds with -O3 and march=native
- Better resource cleanup prevents memory growth
- More efficient CURL handle management

## [2.0.0] - 2024-10-04

### Added
- **Windows GUI Application**: Full Qt-based GUI for Windows with system tray
- **Windows Taskbar Integration**: Real-time speed display in Windows taskbar
- **Advanced Charts**: Real-time bandwidth usage charts with historical data
- **Speed Test Feature**: Built-in network speed testing functionality
- **Data Export**: Export usage statistics to CSV/JSON formats
- **Notification System**: Alerts for data limits and connection issues
- **Dark Mode**: Theme support for better user experience
- **Interface Selection**: Choose specific network interface to monitor
- **Customizable Settings**: Adjustable refresh rates and display options
- **Historical Data View**: Browse and analyze past usage patterns
- **Bandwidth Warnings**: Configurable alerts for speed thresholds

### Enhanced
- **Linux GUI**: Improved GTK dashboard with modern design
- **System Tray**: Enhanced tooltip with more detailed information
- **Data Persistence**: Better storage and retrieval of statistics
- **Performance**: Optimized monitoring for lower resource usage

### Changed
- Version numbering to 2.0.0 for major feature release
- Centralized version management through VERSION file
- Updated documentation with comprehensive feature guides

### Fixed
- Missing systemtray.cpp implementation for Qt version
- Memory leaks in monitoring loops
- UI responsiveness issues

## [1.0.0] - 2024-09-15

### Added
- Cross-compilation support for Windows builds from Linux
- Enhanced dashboard with monthly statistics and data limits
- Data persistence across sessions
- Improved GTK dashboard with monthly stats
- Windows console version with native speed monitoring
- Comprehensive build system for multiple package formats
- Development automation scripts

### Features
- Real-time network speed monitoring
- System tray integration
- GTK dashboard with basic statistics
- Auto-startup configuration
- Multi-distribution packaging support (DEB, RPM, Arch, AppImage)

### Initial Release
- Basic speed monitoring for Linux
- System tray application
- GTK-based dashboard
- Session statistics tracking
